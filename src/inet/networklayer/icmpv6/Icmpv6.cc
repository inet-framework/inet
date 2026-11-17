//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/icmpv6/Icmpv6.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders.h"

namespace inet {

Define_Module(Icmpv6);

namespace {
void writeIcmpv6PseudoHeader(MemoryOutputStream& stream, const Ipv6Address& source, const Ipv6Address& destination, uint32_t length)
{
    for (int i = 0; i < 4; i++) stream.writeUint32Be(source.words()[i]);
    for (int i = 0; i < 4; i++) stream.writeUint32Be(destination.words()[i]);
    stream.writeUint32Be(length);
    stream.writeUint32Be(IP_PROT_IPv6_ICMP);
}

void finalizeIcmpv6Checksum(Packet *packet, b offset, const Ipv6Address& source, const Ipv6Address& destination)
{
    auto length = packet->getDataLength() - offset;
    auto header = packet->removeDataAt<Icmpv6Header>(offset);
    header->setChksum(0);
    MemoryOutputStream stream;
    writeIcmpv6PseudoHeader(stream, source, destination, length.get<B>());
    Chunk::serialize(stream, header);
    if (packet->getDataLength() > offset)
        Chunk::serialize(stream, packet->peekDataAt(offset, packet->getDataLength() - offset));
    header->setChksum(internetChecksum(stream.getData()));
    packet->insertDataAt(header, offset);
}
}


void Icmpv6::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *checksumModeString = par("checksumMode");
        checksumMode = parseChecksumMode(checksumModeString, false);

        WATCH(checksumMode);
        WATCH(pingMap);
        WATCH(transportProtocols);
        WATCH(numEchoReplied);
        WATCH(numErrorsSent);
        WATCH(numErrorsReceived);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_PROTOCOLS) {
        getModuleFromPar<INetfilter>(par("networkProtocolModule"), this)->registerHook(0, this);
        registerService(Protocol::icmpv6, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::icmpv6, gate("ipv6Out"), gate("ipv6In"));
    }
}

void Icmpv6::handleMessageWhenUp(cMessage *msg)
{
    ASSERT(!msg->isSelfMessage()); // no timers in ICMPv6

    // process arriving ICMP message
    if (msg->getArrivalGate()->isName("ipv6In")) {
        if (auto packet = dynamic_cast<Packet *>(msg)) {
            EV_INFO << "Processing ICMPv6 message.\n";
            processICMPv6Message(packet);
        }
        else if (auto indication = dynamic_cast<Indication *>(msg)) {
            // IPv6 reports an ICMPv6 error for an ICMPv6 message we originated (e.g. an
            // error or NDP message that could not be delivered). There is nothing to
            // retransmit, and ICMPv6 must not generate errors about errors, so the
            // notification is simply discarded.
            EV_WARN << "Received an error indication (" << indication->getName()
                    << ") for an ICMPv6 message we sent; ignoring it" << endl;
            delete indication;
        }
        else
            throw cRuntimeError("Unexpected message %s(%s) arrived on ipv6In", msg->getName(), msg->getClassName());
        return;
    }
    else if (msg->getArrivalGate()->isName("transportIn")) {
        // Handle send-error requests from transport layers (e.g. UDP)
        auto request = check_and_cast<Request *>(msg);
        if (auto tag = request->findTagForUpdate<Icmpv6SendErrorReq>()) {
            auto origPacket = tag->getOriginalPacketForUpdate();
            // restore the original network datagram (IP header + transport payload)
            origPacket->setFrontOffset(origPacket->getTag<NetworkProtocolInd>()->getNetworkHeaderFrontOffset());
            sendErrorMessage(origPacket, tag->getType(), tag->getCode());
        }
        else {
            throw cRuntimeError("Unknown Request arrived on transportIn: %s", request->getName());
        }
        delete request;
        return;
    }
    else
        throw cRuntimeError("Message %s(%s) arrived in unknown '%s' gate", msg->getName(), msg->getClassName(), msg->getArrivalGate()->getName());
}

void Icmpv6::processICMPv6Message(Packet *packet)
{
    if (!verifyChecksum(packet)) {
        // drop packet
        EV_WARN << "incoming ICMP packet has wrong checksum, dropped\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    auto icmpv6msg = packet->peekAtFront<Icmpv6Header>();
    int type = icmpv6msg->getType();
    if (type < 128) {
        numErrorsReceived++;
        // ICMPv6 error messages (type < 128).
        // Pop the ICMPv6 header and create an Indication with Icmpv6ErrorInd tag.
        // The remaining packet content (quoted IPv6 + transport + payload) becomes
        // the originalPacket, progressively unwrapped by upper layers.
        const auto& icmpHeader = packet->popAtFront<Icmpv6Header>();

        auto *indication = new Indication("ICMPv6-error");
        auto& errorInd = indication->addTag<Icmpv6ErrorInd>();
        errorInd->setType(icmpHeader->getType());
        // Extract code and MTU from the specific ICMPv6 subtypes
        switch (type) {
            case ICMPv6_DESTINATION_UNREACHABLE:
                errorInd->setCode(CHK(dynamicPtrCast<const Icmpv6DestUnreachableMsg>(icmpHeader))->getCode());
                break;
            case ICMPv6_PACKET_TOO_BIG: {
                auto ptb = CHK(dynamicPtrCast<const Icmpv6PacketTooBigMsg>(icmpHeader));
                errorInd->setCode(ptb->getCode());
                errorInd->setMtu(ptb->getMTU());
                break;
            }
            case ICMPv6_TIME_EXCEEDED:
                errorInd->setCode(CHK(dynamicPtrCast<const Icmpv6TimeExceededMsg>(icmpHeader))->getCode());
                break;
            case ICMPv6_PARAMETER_PROBLEM:
                errorInd->setCode(CHK(dynamicPtrCast<const Icmpv6ParamProblemMsg>(icmpHeader))->getCode());
                break;
        }
        packet->trim();
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv6);
        errorInd->setOriginalPacket(packet); // ownership transfer, no dup needed

        // Peek at the quoted IPv6 header to determine the transport protocol
        const auto& bogusIpv6Header = packet->peekAtFront<Ipv6Header>();
        int transportProtocol = bogusIpv6Header->getProtocolId();
        if (transportProtocol == IP_PROT_IPv6_ICMP) {
            // ICMP error answer to an ICMP packet:
            errorOut(indication);
        }
        else if (!contains(transportProtocols, transportProtocol)) {
            // Nothing registered for the protocol the quoted datagram names, so there is
            // nobody to hand the report to. Sending it anyway reaches a MessageDispatcher
            // that knows no route for it and stops the run. Icmp does the same check.
            EV_ERROR << "Transport protocol " << transportProtocol << " not registered, packet dropped\n";
            delete indication;
        }
        else {
            // Send the Indication to IPv6 via ipv6Out; IPv6 will pop the quoted
            // IPv6 header and forward the indication to the transport protocol.
            indication->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
            send(indication, "ipv6Out");
        }
    }
    else {
        switch (type) {
            case ICMPv6_ECHO_REQUEST: {
                EV_INFO << "ICMPv6 Echo Request Message Received." << endl;
                const auto& echoRequest = packet->popAtFront<Icmpv6EchoRequestMsg>();
                processEchoRequest(packet, echoRequest);
                break;
            }
            case ICMPv6_ECHO_REPLY: {
                EV_INFO << "ICMPv6 Echo Reply Message Received." << endl;
                const auto& echoReply = packet->popAtFront<Icmpv6EchoReplyMsg>();
                processEchoReply(packet, echoReply);
                break;
            }
            case ICMPv6_MLD_QUERY:
            case ICMPv6_MLD_REPORT:
            case ICMPv6_MLD_DONE:
            case ICMPv6_MLDv2_REPORT: {
                // MLD messages (RFC 2710 / RFC 3810) are ICMPv6 subtypes: Query (130),
                // MLDv1 Report (131), Done (132), and MLDv2 Report (143). If this node runs
                // MLD, forward to the MLD module via the lp dispatcher by re-tagging with
                // Protocol::mld. If MLD is not enabled (no mldModule configured), a node
                // not participating in MLD silently ignores the message.
                if (par("mldModule").stringValue()[0] == '\0') {
                    EV_INFO << "ICMPv6: received MLD message (type=" << type << ") but MLD is not enabled on this node; dropping.\n";
                    PacketDropDetails details;
                    details.setReason(NO_PROTOCOL_FOUND);
                    emit(packetDroppedSignal, packet, &details);
                    delete packet;
                    break;
                }
                EV_INFO << "ICMPv6: forwarding MLD message (type=" << type << ") to MLD module.\n";
                packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::mld);
                packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mld);
                send(packet, "ipv6Out");
                break;
            }
            default: {
                // RFC 4443 section 2.4(b): a node MUST silently discard an ICMPv6
                // informational message of a type it does not recognize. Stopping the
                // simulation let any peer end the run with one packet.
                EV_WARN << "Unknown ICMPv6 message type " << type << ", packet dropped\n";
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, packet, &details);
                delete packet;
                break;
            }
        }
    }
}

/*
 * RFC 4443 4.2:
 *
 * Every node MUST implement an ICMPv6 Echo responder function that
 * receives Echo Requests and originates corresponding Echo Replies.  A
 * node SHOULD also implement an application-layer interface for
 * originating Echo Requests and receiving Echo Replies, for diagnostic
 * purposes.
 *
 * The source address of an Echo Reply sent in response to a unicast
 * Echo Request message MUST be the same as the destination address of
 * that Echo Request message.
 *
 * An Echo Reply SHOULD be sent in response to an Echo Request message
 * sent to an Ipv6 multicast or anycast address.  In this case, the
 * source address of the reply MUST be a unicast address belonging to
 * the interface on which the Echo Request message was received.
 *
 * The data received in the ICMPv6 Echo Request message MUST be returned
 * entirely and unmodified in the ICMPv6 Echo Reply message.
 */
void Icmpv6::processEchoRequest(Packet *requestPacket, const Ptr<const Icmpv6EchoRequestMsg>& requestHeader)
{
    // Create an ICMPv6 Reply Message
    auto replyPacket = new Packet();
    replyPacket->setName((std::string(requestPacket->getName()) + "-reply").c_str());
    auto replyHeader = makeShared<Icmpv6EchoReplyMsg>();
    replyHeader->setIdentifier(requestHeader->getIdentifier());
    replyHeader->setSeqNumber(requestHeader->getSeqNumber());
    replyPacket->insertAtBack(requestPacket->peekData());
    insertChecksum(replyHeader, replyPacket);
    replyPacket->insertAtFront(replyHeader);

    auto addressInd = requestPacket->getTag<L3AddressInd>();
    replyPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
    auto addressReq = replyPacket->addTag<L3AddressReq>();
    addressReq->setDestAddress(addressInd->getSrcAddress());

    if (addressInd->getDestAddress().isMulticast() /*TODO check for anycast too*/) {
        IInterfaceTable *it = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        auto ipv6Data = it->getInterfaceById(requestPacket->getTag<InterfaceInd>()->getInterfaceId())->getProtocolDataForUpdate<Ipv6InterfaceData>();
        addressReq->setSrcAddress(ipv6Data->getPreferredAddress());
        // TODO implement default address selection properly.
        //      According to RFC 3484, the source address to be used
        //      depends on the destination address
    }
    else
        addressReq->setSrcAddress(addressInd->getDestAddress());

    delete requestPacket;
    numEchoReplied++;
    sendToIP(replyPacket);
}

void Icmpv6::processEchoReply(Packet *packet, const Ptr<const Icmpv6EchoReplyMsg>& reply)
{
    delete packet;
}

void Icmpv6::sendErrorMessage(Packet *origDatagram, Icmpv6Type type, int code, int mtu)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // Packets received from the network have an InterfaceInd tag;
    // locally originated packets don't. Skip validation for locally
    // originated packets so the error can be processed locally (see below).
    bool fromNetwork = origDatagram->findTag<InterfaceInd>() != nullptr;

    if (fromNetwork && !validateDatagramPromptingError(origDatagram))
        return;

    Packet *errorMsg;

    if (type == ICMPv6_DESTINATION_UNREACHABLE)
        errorMsg = createDestUnreachableMsg(static_cast<Icmpv6DestUnav>(code));
    else if (type == ICMPv6_PACKET_TOO_BIG)
        // RFC 4443 section 3.2: the MTU field carries the MTU of the next-hop link. It
        // used to be a literal zero, which names no link and tells the source nothing.
        errorMsg = createPacketTooBigMsg(mtu);
    else if (type == ICMPv6_TIME_EXCEEDED)
        errorMsg = createTimeExceededMsg(static_cast<Icmpv6TimeEx>(code));
    else if (type == ICMPv6_PARAMETER_PROBLEM)
        errorMsg = createParamProblemMsg(static_cast<Icmpv6ParameterProblem>(code));
    else
        throw cRuntimeError("Unknown ICMPv6 error type: %d\n", type);

    // RFC 4443 section 2.4(c): include the outer IPv6 header in the minimum-MTU
    // budget, otherwise the error itself requires IPv6 source fragmentation.
    b copyLength = B(IPv6_MIN_MTU) - IPv6_HEADER_BYTES - errorMsg->getDataLength();
    errorMsg->insertAtBack(origDatagram->peekDataAt(b(0), std::min(copyLength, origDatagram->getDataLength())));

    auto icmpHeader = errorMsg->removeAtFront<Icmpv6Header>();
    insertChecksum(icmpHeader, errorMsg);
    errorMsg->insertAtFront(icmpHeader);

    // debugging information
    EV_DEBUG << "sending ICMP error: (" << errorMsg->getClassName() << ")" << errorMsg->getName()
             << " type=" << type << " code=" << code << endl;

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    const auto& ipv6Header = origDatagram->peekAtFront<Ipv6Header>();
    numErrorsSent++;
    if (ipv6Header->getSrcAddress().isUnspecified()) {
        // pretend it came from the IP layer
        errorMsg->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
        auto addresses = errorMsg->addTag<L3AddressInd>();
        addresses->setSrcAddress(Ipv6Address::LOOPBACK_ADDRESS);
        addresses->setDestAddress(Ipv6Address::LOOPBACK_ADDRESS);
        // This local indication bypasses IPv6 egress and therefore its checksum hook.
        if (errorMsg->peekAtFront<Icmpv6Header>()->getChecksumMode() == CHECKSUM_COMPUTED)
            finalizeIcmpv6Checksum(errorMsg, b(0), Ipv6Address::LOOPBACK_ADDRESS, Ipv6Address::LOOPBACK_ADDRESS);

        // then process it locally
        processICMPv6Message(errorMsg);
    }
    else {
        sendToIP(errorMsg, ipv6Header->getSrcAddress());
    }
}

void Icmpv6::sendToIP(Packet *msg, const Ipv6Address& dest)
{
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(dest);
    sendToIP(msg);
}

void Icmpv6::sendToIP(Packet *msg)
{
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
    send(msg, "ipv6Out");
}

Packet *Icmpv6::createDestUnreachableMsg(Icmpv6DestUnav code)
{
    auto errorMsg = makeShared<Icmpv6DestUnreachableMsg>();
    errorMsg->setType(ICMPv6_DESTINATION_UNREACHABLE);
    errorMsg->setCode(code);
    auto packet = new Packet("Dest Unreachable");
    packet->insertAtBack(errorMsg);
    return packet;
}

Packet *Icmpv6::createPacketTooBigMsg(int mtu)
{
    auto errorMsg = makeShared<Icmpv6PacketTooBigMsg>();
    errorMsg->setType(ICMPv6_PACKET_TOO_BIG);
    errorMsg->setCode(0); // Set to 0 by sender and ignored by receiver.
    errorMsg->setMTU(mtu);
    auto packet = new Packet("Packet Too Big");
    packet->insertAtBack(errorMsg);
    return packet;
}

Packet *Icmpv6::createTimeExceededMsg(Icmpv6TimeEx code)
{
    auto errorMsg = makeShared<Icmpv6TimeExceededMsg>();
    errorMsg->setType(ICMPv6_TIME_EXCEEDED);
    errorMsg->setCode(code);
    auto packet = new Packet("Time Exceeded");
    packet->insertAtBack(errorMsg);
    return packet;
}

Packet *Icmpv6::createParamProblemMsg(Icmpv6ParameterProblem code)
{
    auto errorMsg = makeShared<Icmpv6ParamProblemMsg>();
    errorMsg->setType(ICMPv6_PARAMETER_PROBLEM);
    errorMsg->setCode(code);
    // TODO What Pointer? section 3.4
    auto packet = new Packet("Parameter Problem");
    packet->insertAtBack(errorMsg);
    return packet;
}

bool Icmpv6::validateDatagramPromptingError(Packet *packet)
{
    auto ipv6Header = packet->peekAtFront<Ipv6Header>();
    // don't send ICMP error messages for multicast messages
    if (ipv6Header->getDestAddress().isMulticast()) {
        EV_INFO << "won't send ICMP error messages for multicast message " << ipv6Header << endl;
        return false;
    }

    // RFC 4443 Section 2.4(e): don't send ICMPv6 error if source is unspecified or multicast
    if (ipv6Header->getSrcAddress().isUnspecified()) {
        EV_INFO << "won't send ICMP error messages to unspecified address, message " << ipv6Header << endl;
        return false;
    }
    if (ipv6Header->getSrcAddress().isMulticast()) {
        EV_INFO << "won't send ICMP error messages to multicast address, message " << ipv6Header << endl;
        return false;
    }

    // RFC 4443 section 2.4(e): no ICMPv6 error message about a packet that arrived as a
    // link-layer broadcast or multicast. The conditions above read the IPv6 addresses; this
    // one reads the frame the packet arrived in, whose destination the IPv6 header does not
    // record.
    if (auto& macAddressInd = packet->findTag<MacAddressInd>()) {
        const MacAddress& destMacAddress = macAddressInd->getDestAddress();
        if (destMacAddress.isBroadcast() || destMacAddress.isMulticast()) {
            EV_INFO << "won't send ICMP error messages for a link-layer broadcast or multicast, message " << ipv6Header << endl;
            return false;
        }
    }

    // do not reply with error message to error message
    if (ipv6Header->getProtocolId() == IP_PROT_IPv6_ICMP) {
        auto recICMPMsg = packet->peekDataAt<Icmpv6Header>(ipv6Header->getChunkLength());
        if (recICMPMsg->getType() < 128) {
            EV_INFO << "ICMP error received -- do not reply to it" << endl;
            return false;
        }
    }
    return true;
}

void Icmpv6::errorOut(Indication *indication)
{
    delete indication;
}

void Icmpv6::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void Icmpv6::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("transportOut", gate->getBaseName())) {
        int protocolNumber = ProtocolGroup::getIpProtocolGroup()->findProtocolNumber(&protocol);
        if (protocolNumber != -1)
            transportProtocols.insert(protocolNumber);
    }
}

void Icmpv6::insertChecksum(ChecksumMode checksumMode, const Ptr<Icmpv6Header>& icmpHeader, Packet *packet)
{
    icmpHeader->setChecksumMode(checksumMode);
    switch (checksumMode) {
        case CHECKSUM_DECLARED_CORRECT:
            // if the checksum mode is declared to be correct, then set the checksum to an easily recognizable value
            icmpHeader->setChksum(0xC00D);
            break;
        case CHECKSUM_DECLARED_INCORRECT:
            // if the checksum mode is declared to be incorrect, then set the checksum to an easily recognizable value
            icmpHeader->setChksum(0xBAAD);
            break;
        case CHECKSUM_COMPUTED:
            // Source selection has not happened yet. POST_ROUTING finalizes the
            // RFC 4443 pseudo-header checksum before source fragmentation.
            icmpHeader->setChksum(0);
            break;
        default:
            throw cRuntimeError("Unknown checksum mode %d", (int)checksumMode);
    }
}


INetfilter::IHook::Result Icmpv6::datagramPostRoutingHook(Packet *packet)
{
    Enter_Method("datagramPostRoutingHook");
    if (packet->findTag<InterfaceInd>()) return ACCEPT;
    auto ipv6 = packet->peekAtFront<Ipv6Header>();
    auto nextHeader = ipv6->getProtocolId();
    b offset = ipv6->getChunkLength();
    bool activeRouting = false;
    while (isIpv6ExtensionHeader(nextHeader)) {
        // Locally prefragmented input must already carry its whole-message checksum.
        if (nextHeader == IP_PROT_IPv6EXT_FRAGMENT) return ACCEPT;
        auto extension = peekIpv6ExtensionHeaderAt(packet, offset, nextHeader);
        if (nextHeader == IP_PROT_IPv6EXT_ROUTING)
            activeRouting |= staticPtrCast<const Ipv6RoutingHeader>(extension)->getSegmentsLeft() != 0;
        offset += extension->getChunkLength();
        nextHeader = extension->getNextHeaderProtocol();
    }
    if (nextHeader != IP_PROT_IPv6_ICMP) return ACCEPT;
    auto existing = packet->peekDataAt<Icmpv6Header>(offset);
    if (existing->getChecksumMode() != CHECKSUM_COMPUTED) return ACCEPT;
    if (activeRouting)
        throw cRuntimeError("Computed ICMPv6 with active routing-header destination semantics is unsupported");
    finalizeIcmpv6Checksum(packet, offset, ipv6->getSrcAddress(), ipv6->getDestAddress());
    return ACCEPT;
}

bool Icmpv6::verifyChecksum(const Packet *packet)
{
    const auto& icmpHeader = packet->peekAtFront<Icmpv6Header>(b(-1), Chunk::PF_ALLOW_INCORRECT);
    switch (icmpHeader->getChecksumMode()) {
        case CHECKSUM_DECLARED_CORRECT:
            // if the checksum mode is declared to be correct, then the check passes if and only if the chunks are correct
            return icmpHeader->isCorrect();
        case CHECKSUM_DECLARED_INCORRECT:
            // if the checksum mode is declared to be incorrect, then the check fails
            return false;
        case CHECKSUM_COMPUTED: {
            // otherwise compute the checksum, the check passes if the result is 0xFFFF (includes the received checksum)
            auto dataBytes = packet->peekDataAsBytes(Chunk::PF_ALLOW_INCORRECT);
            auto addresses = packet->findTag<L3AddressInd>();
            if (!addresses || addresses->getSrcAddress().getType() != L3Address::IPv6 || addresses->getDestAddress().getType() != L3Address::IPv6)
                return false;
            MemoryOutputStream stream;
            writeIcmpv6PseudoHeader(stream, addresses->getSrcAddress().toIpv6(), addresses->getDestAddress().toIpv6(), packet->getDataLength().get<B>());
            stream.writeBytes(dataBytes->getBytes());
            uint16_t checksum = internetChecksum(stream.getData());
            // TODO delete these isCorrect calls, rely on checksum only
            return checksum == 0 && icmpHeader->isCorrect();
        }
        default:
            throw cRuntimeError("Unknown checksum mode %d", (int)icmpHeader->getChecksumMode());
    }
}

} // namespace inet
