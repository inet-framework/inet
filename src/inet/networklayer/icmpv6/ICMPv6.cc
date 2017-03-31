//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/networklayer/icmpv6/ICMPv6.h"

#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"

#include "inet/linklayer/common/InterfaceTag_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/ICMPv6Header_m.h"
#include "inet/networklayer/ipv6/IPv6Header.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"


namespace inet {

Define_Module(ICMPv6);

void ICMPv6::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        registerProtocol(Protocol::icmpv6, gate("ipv6Out"));
        registerProtocol(Protocol::icmpv6, gate("transportOut"));
    }
}

void ICMPv6::handleMessage(cMessage *msg)
{
    ASSERT(!msg->isSelfMessage());    // no timers in ICMPv6

    // process arriving ICMP message
    if (msg->getArrivalGate()->isName("ipv6In")) {
        EV_INFO << "Processing ICMPv6 message.\n";
        processICMPv6Message(check_and_cast<Packet *>(msg));
        return;
    }
    else
        throw cRuntimeError("Message %s(%s) arrived in unknown '%s' gate", msg->getName(), msg->getClassName(), msg->getArrivalGate()->getName());
}

void ICMPv6::processICMPv6Message(Packet *packet)
{
    auto icmpv6msg = packet->peekHeader<ICMPv6Header>();
    int type = icmpv6msg->getType();
    if (type < 128) {
        // ICMP errors are delivered to the appropriate higher layer protocols
        EV_INFO << "ICMPv6 packet: passing it to higher layer\n";
        const auto& bogusIpv6Header = packet->peekDataAt<IPv6Header>(icmpv6msg->getChunkLength());
        int transportProtocol = bogusIpv6Header->getTransportProtocol();
        if (transportProtocol == IP_PROT_IPv6_ICMP) {
            // ICMP error answer to an ICMP packet:
            errorOut(icmpv6msg);
            delete packet;
        }
        else {
            packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ipprotocol.getProtocol(transportProtocol));
            send(packet, "transportOut");
        }
    }
    else {
        auto icmpv6msg = packet->popHeader<ICMPv6Header>();
        if (std::dynamic_pointer_cast<ICMPv6DestUnreachableMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Destination Unreachable Message Received." << endl;
            errorOut(icmpv6msg);
            delete packet;
        }
        else if (std::dynamic_pointer_cast<ICMPv6PacketTooBigMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Packet Too Big Message Received." << endl;
            errorOut(icmpv6msg);
            delete packet;
        }
        else if (std::dynamic_pointer_cast<ICMPv6TimeExceededMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Time Exceeded Message Received." << endl;
            errorOut(icmpv6msg);
            delete packet;
        }
        else if (std::dynamic_pointer_cast<ICMPv6ParamProblemMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Parameter Problem Message Received." << endl;
            errorOut(icmpv6msg);
        }
        else if (auto echoRequest = std::dynamic_pointer_cast<ICMPv6EchoRequestMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Echo Request Message Received." << endl;
            processEchoRequest(packet, echoRequest);
        }
        else if (auto echoReply = std::dynamic_pointer_cast<ICMPv6EchoReplyMsg>(icmpv6msg)) {
            EV_INFO << "ICMPv6 Echo Reply Message Received." << endl;
            processEchoReply(packet, echoReply);
        }
        else
            throw cRuntimeError("Unknown message type received: (%s)%s.\n", icmpv6msg->getClassName(),icmpv6msg->getName());
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
 * sent to an IPv6 multicast or anycast address.  In this case, the
 * source address of the reply MUST be a unicast address belonging to
 * the interface on which the Echo Request message was received.
 *
 * The data received in the ICMPv6 Echo Request message MUST be returned
 * entirely and unmodified in the ICMPv6 Echo Reply message.
 */
void ICMPv6::processEchoRequest(Packet *requestPacket, const std::shared_ptr<ICMPv6EchoRequestMsg>& requestHeader)
{
    //Create an ICMPv6 Reply Message
    auto replyPacket = new Packet("Echo Reply");
    replyPacket->setName((std::string(requestHeader->getName()) + "-reply").c_str());
    replyPacket->append(requestPacket->peekDataAt(bit(0), requestPacket->getDataLength()));
    auto reply = std::make_shared<ICMPv6EchoReplyMsg>();
    reply->setType(ICMPv6_ECHO_REPLY);

    auto addressInd = requestPacket->getMandatoryTag<L3AddressInd>();
    replyPacket->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
    auto addressReq = replyPacket->ensureTag<L3AddressReq>();
    addressReq->setDestAddress(addressInd->getSrcAddress());

    if (addressInd->getDestAddress().isMulticast()    /*TODO check for anycast too*/) {
        IInterfaceTable *it = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        IPv6InterfaceData *ipv6Data = it->getInterfaceById(requestPacket->getMandatoryTag<InterfaceInd>()->getInterfaceId())->ipv6Data();
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

void ICMPv6::processEchoReply(Packet *packet, const std::shared_ptr<ICMPv6EchoReplyMsg>& reply)
{
    delete packet;
}

void ICMPv6::sendErrorMessage(Packet *origDatagram, ICMPv6Type type, int code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // get ownership
    take(origDatagram);

    if (!validateDatagramPromptingError(origDatagram))
        return;

    Packet *errorMsg;

    if (type == ICMPv6_DESTINATION_UNREACHABLE)
        errorMsg = createDestUnreachableMsg(code);
    //TODO: implement MTU support.
    else if (type == ICMPv6_PACKET_TOO_BIG)
        errorMsg = createPacketTooBigMsg(0);
    else if (type == ICMPv6_TIME_EXCEEDED)
        errorMsg = createTimeExceededMsg(code);
    else if (type == ICMPv6_PARAMETER_PROBLEM)
        errorMsg = createParamProblemMsg(code);
    else
        throw cRuntimeError("Unknown ICMPv6 error type: %d\n", type);

    // Encapsulate the original datagram, but the whole ICMPv6 error
    // packet cannot be larger than the minimum IPv6 MTU (RFC 4443 2.4. (c)).
    // NOTE: since we just overwrite the errorMsg length without actually
    // truncating origDatagram, one can get "packet length became negative"
    // error when decapsulating the origDatagram on the receiver side.
    // A workaround is to avoid decapsulation, or to manually set the
    // errorMessage length to be larger than the encapsulated message.
    bit copyLength =  byte(IPv6_MIN_MTU) - errorMsg->getTotalLength();
    errorMsg->append(origDatagram->peekDataAt(bit(0), std::min(copyLength, origDatagram->getDataLength())));

    // debugging information
    EV_DEBUG << "sending ICMP error: (" << errorMsg->getClassName() << ")" << errorMsg->getName()
             << " type=" << type << " code=" << code << endl;

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    const auto& ipv6Header = origDatagram->peekHeader<IPv6Header>();
    if (ipv6Header->getSrcAddress().isUnspecified()) {
        // pretend it came from the IP layer
        errorMsg->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);
        errorMsg->ensureTag<L3AddressInd>()->setSrcAddress(IPv6Address::LOOPBACK_ADDRESS);    // FIXME maybe use configured loopback address

        // then process it locally
        processICMPv6Message(errorMsg);
    }
    else {
        sendToIP(errorMsg, ipv6Header->getSrcAddress());
    }
}

void ICMPv6::sendToIP(Packet *msg, const IPv6Address& dest)
{
    msg->ensureTag<L3AddressReq>()->setDestAddress(dest);
    msg->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv6);

    send(msg, "ipv6Out");
}

void ICMPv6::sendToIP(Packet *msg)
{
    // assumes IPv6ControlInfo is already attached
    send(msg, "ipv6Out");
}

Packet *ICMPv6::createDestUnreachableMsg(int code)
{
    auto errorMsg = std::make_shared<ICMPv6DestUnreachableMsg>();
    errorMsg->setType(ICMPv6_DESTINATION_UNREACHABLE);
    errorMsg->setCode(code);
    auto packet = new Packet("Dest Unreachable");
    errorMsg->markImmutable();
    packet->append(errorMsg);
    return packet;
}

Packet *ICMPv6::createPacketTooBigMsg(int mtu)
{
    auto errorMsg = std::make_shared<ICMPv6PacketTooBigMsg>();
    errorMsg->setType(ICMPv6_PACKET_TOO_BIG);
    errorMsg->setCode(0);    //Set to 0 by sender and ignored by receiver.
    errorMsg->setMTU(mtu);
    auto packet = new Packet("Packet Too Big");
    errorMsg->markImmutable();
    packet->append(errorMsg);
    return packet;
}

Packet *ICMPv6::createTimeExceededMsg(int code)
{
    auto errorMsg = std::make_shared<ICMPv6TimeExceededMsg>();
    errorMsg->setType(ICMPv6_TIME_EXCEEDED);
    errorMsg->setCode(code);
    auto packet = new Packet("Time Exceeded");
    errorMsg->markImmutable();
    packet->append(errorMsg);
    return packet;
}

Packet *ICMPv6::createParamProblemMsg(int code)
{
    auto errorMsg = std::make_shared<ICMPv6ParamProblemMsg>();
    errorMsg->setType(ICMPv6_PARAMETER_PROBLEM);
    errorMsg->setCode(code);
    //TODO: What Pointer? section 3.4
    auto packet = new Packet("Parameter Problem");
    errorMsg->markImmutable();
    packet->append(errorMsg);
    return packet;
}

bool ICMPv6::validateDatagramPromptingError(Packet *packet)
{
    auto ipv6Header = packet->peekHeader<IPv6Header>();
    // don't send ICMP error messages for multicast messages
    if (ipv6Header->getDestAddress().isMulticast()) {
        EV_INFO << "won't send ICMP error messages for multicast message " << ipv6Header << endl;
        delete packet;
        return false;
    }

    // do not reply with error message to error message
    if (ipv6Header->getTransportProtocol() == IP_PROT_IPv6_ICMP) {
        auto recICMPMsg = packet->peekDataAt<ICMPv6Header>(ipv6Header->getChunkLength());
        if (recICMPMsg->getType() < 128) {
            EV_INFO << "ICMP error received -- do not reply to it" << endl;
            delete packet;
            return false;
        }
    }
    return true;
}

void ICMPv6::errorOut(const std::shared_ptr<ICMPv6Header>& icmpv6msg)
{
}

bool ICMPv6::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    //pingMap.clear();
    throw cRuntimeError("Lifecycle operation support not implemented");
}

void ICMPv6::handleRegisterProtocol(const Protocol& protocol, cGate *gate)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("transportIn", gate->getBaseName())) {
        transportProtocols.insert(ProtocolGroup::ipprotocol.getProtocolNumber(&protocol));
    }
}

} // namespace inet

