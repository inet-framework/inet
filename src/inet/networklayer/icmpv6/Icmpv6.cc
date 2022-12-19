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
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

namespace inet {

Define_Module(Icmpv6);

void Icmpv6::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_PROTOCOLS) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(getContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        registerService(Protocol::icmpv6, gate("transportIn"), gate("transportOut"));
        registerProtocol(Protocol::icmpv6, gate("ipv6Out"), gate("ipv6In"));
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
    else
        throw cRuntimeError("Message %s(%s) arrived in unknown '%s' gate", msg->getName(), msg->getClassName(), msg->getArrivalGate()->getName());
}

void Icmpv6::processICMPv6Message(Packet *packet)
{
    if (!verifyCrc(packet)) {
        // drop packet
        EV_WARN << "incoming ICMP packet has wrong CRC, dropped\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    auto icmpv6msg = packet->peekAtFront<Icmpv6Header>();
    int type = icmpv6msg->getType();
    if (type < 128) {
        // ICMP errors are delivered to the appropriate higher layer protocols
        EV_INFO << "ICMPv6 packet: passing it to higher layer\n";
        const auto& bogusIpv6Header = packet->peekDataAt<Ipv6Header>(icmpv6msg->getChunkLength());
        int transportProtocol = bogusIpv6Header->getProtocolId();
        if (transportProtocol == IP_PROT_IPv6_ICMP) {
            // ICMP error answer to an ICMP packet:
            errorOut(icmpv6msg);
            delete packet;
        }
        else {
            auto dispatchProtocolReq = packet->addTagIfAbsent<DispatchProtocolReq>();
            dispatchProtocolReq->setServicePrimitive(SP_INDICATION);
            dispatchProtocolReq->setProtocol(ProtocolGroup::getIpProtocolGroup()->getProtocol(transportProtocol));
            send(packet, "transportOut");
        }
    }
    else {
        auto icmpv6msg = packet->popAtFront<Icmpv6Header>();
        if (dynamicPtrCast<const Icmpv6DestUnreachableMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Destination Unreachable Message Received." << endl;
            errorOut(icmpv6msg);
            delete packet;
        }
        else if (dynamicPtrCast<const Icmpv6PacketTooBigMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Packet Too Big Message Received." << endl;
            errorOut(icmpv6msg);
            delete packet;
        }
        else if (dynamicPtrCast<const Icmpv6TimeExceededMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Time Exceeded Message Received." << endl;
            errorOut(icmpv6msg);
            delete packet;
        }
        else if (dynamicPtrCast<const Icmpv6ParamProblemMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Parameter Problem Message Received." << endl;
            errorOut(icmpv6msg);
            delete packet;
        }
        else if (auto echoRequest = dynamicPtrCast<const Icmpv6EchoRequestMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Echo Request Message Received." << endl;
            processEchoRequest(packet, echoRequest);
        }
        else if (auto echoReply = dynamicPtrCast<const Icmpv6EchoReplyMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Echo Reply Message Received." << endl;
            processEchoReply(packet, echoReply);
        }
        else
            throw cRuntimeError("Unknown message type received: (%s)%s.\n", icmpv6msg->getClassName(), icmpv6msg->getName());
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
    insertCrc(replyHeader, replyPacket);
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

    if (!validateDatagramPromptingError(origDatagram))
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
    b copyLength = B(IPv6_MIN_MTU) - errorMsg->getTotalLength();
    errorMsg->insertAtBack(origDatagram->peekDataAt(b(0), std::min(copyLength, origDatagram->getDataLength())));

    auto icmpHeader = errorMsg->removeAtFront<Icmpv6Header>();
    insertCrc(icmpHeader, errorMsg);
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

void Icmpv6::errorOut(const Ptr<const Icmpv6Header>& icmpv6msg)
{
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

void Icmpv6::insertCrc(CrcMode crcMode, const Ptr<Icmpv6Header>& icmpHeader, Packet *packet)
{
    icmpHeader->setCrcMode(crcMode);
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            icmpHeader->setChksum(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            icmpHeader->setChksum(0xBAAD);
            break;
        case CRC_COMPUTED: {
            // if the CRC mode is computed, then compute the CRC and set it
            icmpHeader->setChksum(0x0000); // make sure that the CRC is 0 in the header before computing the CRC
            MemoryOutputStream icmpStream;
            Chunk::serialize(icmpStream, icmpHeader);
            if (packet->getByteLength() > 0)
                Chunk::serialize(icmpStream, packet->peekDataAsBytes());
            uint16_t crc = TcpIpChecksum::checksum(icmpStream.getData());
            icmpHeader->setChksum(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

bool Icmpv6::verifyCrc(const Packet *packet)
{
    const auto& icmpHeader = packet->peekAtFront<Icmpv6Header>(b(-1), Chunk::PF_ALLOW_INCORRECT);
    switch (icmpHeader->getCrcMode()) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then the check passes if and only if the chunks are correct
            return icmpHeader->isCorrect();
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then the check fails
            return false;
        case CRC_COMPUTED: {
            // otherwise compute the CRC, the check passes if the result is 0xFFFF (includes the received CRC)
            auto dataBytes = packet->peekDataAsBytes(Chunk::PF_ALLOW_INCORRECT);
            uint16_t crc = TcpIpChecksum::checksum(dataBytes->getBytes());
            // TODO delete these isCorrect calls, rely on CRC only
            return crc == 0 && icmpHeader->isCorrect();
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

} // namespace inet

