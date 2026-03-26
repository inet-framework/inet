//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/icmpv6/Icmpv6.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

Define_Module(Icmpv6);

void Icmpv6::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *checksumModeString = par("checksumMode");
        checksumMode = parseChecksumMode(checksumModeString, false);
        ipv6Sink.reference(gate("ipv6Out"), true);
        transportSink.reference(gate("transportOut"), true);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_PROTOCOLS) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(getContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void Icmpv6::handleMessage(cMessage *msg)
{
    ASSERT(!msg->isSelfMessage()); // no timers in ICMPv6

    // process arriving ICMP message
    if (msg->getArrivalGate()->isName("ipv6In")) {
        EV_INFO << "Processing ICMPv6 message.\n";
        processICMPv6Message(check_and_cast<Packet *>(msg));
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
            default:
                throw cRuntimeError("Unknown ICMPv6 message type %d received", type);
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
    sendToIP(replyPacket);
}

void Icmpv6::processEchoReply(Packet *packet, const Ptr<const Icmpv6EchoReplyMsg>& reply)
{
    delete packet;
}

void Icmpv6::sendErrorMessage(Packet *origDatagram, Icmpv6Type type, int code)
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
    // TODO implement MTU support.
    else if (type == ICMPv6_PACKET_TOO_BIG)
        errorMsg = createPacketTooBigMsg(0);
    else if (type == ICMPv6_TIME_EXCEEDED)
        errorMsg = createTimeExceededMsg(static_cast<Icmpv6TimeEx>(code));
    else if (type == ICMPv6_PARAMETER_PROBLEM)
        errorMsg = createParamProblemMsg(static_cast<Icmpv6ParameterProblem>(code));
    else
        throw cRuntimeError("Unknown ICMPv6 error type: %d\n", type);

    // Encapsulate the original datagram, but the whole ICMPv6 error
    // packet cannot be larger than the minimum Ipv6 MTU (RFC 4443 2.4. (c)).
    // NOTE: since we just overwrite the errorMsg length without actually
    // truncating origDatagram, one can get "packet length became negative"
    // error when decapsulating the origDatagram on the receiver side.
    // A workaround is to avoid decapsulation, or to manually set the
    // errorMessage length to be larger than the encapsulated message.
    b copyLength = B(IPv6_MIN_MTU) - errorMsg->getDataLength();
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
    if (ipv6Header->getSrcAddress().isUnspecified()) {
        // pretend it came from the IP layer
        errorMsg->addTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
        errorMsg->addTag<L3AddressInd>()->setSrcAddress(Ipv6Address::LOOPBACK_ADDRESS); // FIXME maybe use configured loopback address

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
    yieldBeforePush();
    ipv6Sink.pushPacket(msg);
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
        case CHECKSUM_COMPUTED: {
            // if the checksum mode is computed, then compute the checksum and set it
            icmpHeader->setChksum(0x0000); // make sure that the checksum is 0 in the header before computing the checksum
            MemoryOutputStream icmpStream;
            Chunk::serialize(icmpStream, icmpHeader);
            if (packet->getByteLength() > 0)
                Chunk::serialize(icmpStream, packet->peekDataAsBytes());
            uint16_t checksum = internetChecksum(icmpStream.getData());
            icmpHeader->setChksum(checksum);
            break;
        }
        default:
            throw cRuntimeError("Unknown checksum mode");
    }
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
            uint16_t checksum = internetChecksum(dataBytes->getBytes());
            // TODO delete these isCorrect calls, rely on checksum only
            return checksum == 0 && icmpHeader->isCorrect();
        }
        default:
            throw cRuntimeError("Unknown checksum mode");
    }
}

void Icmpv6::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    processICMPv6Message(packet);
}

} // namespace inet

