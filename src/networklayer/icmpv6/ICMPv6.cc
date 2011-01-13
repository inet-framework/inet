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

#include <omnetpp.h>
#include "ICMPv6.h"


Define_Module(ICMPv6);


void ICMPv6::initialize()
{
}

void ICMPv6::handleMessage(cMessage *msg)
{
    ASSERT(!msg->isSelfMessage()); // no timers in ICMPv6

    // process arriving ICMP message
    if (msg->getArrivalGate()->isName("ipv6In"))
    {
        EV << "Processing ICMPv6 message.\n";
        processICMPv6Message(check_and_cast<ICMPv6Message *>(msg));
        return;
    }

    // request from application
    if (msg->getArrivalGate()->isName("pingIn"))
    {
        sendEchoRequest(PK(msg));
        return;
    }
}

void ICMPv6::processICMPv6Message(ICMPv6Message *icmpv6msg)
{
    ASSERT(dynamic_cast<ICMPv6Message *>(icmpv6msg));
    if (dynamic_cast<ICMPv6DestUnreachableMsg *>(icmpv6msg))
    {
        EV << "ICMPv6 Destination Unreachable Message Received." << endl;
        errorOut(icmpv6msg);
    }
    else if (dynamic_cast<ICMPv6PacketTooBigMsg *>(icmpv6msg))
    {
        EV << "ICMPv6 Packet Too Big Message Received." << endl;
        errorOut(icmpv6msg);
    }
    else if (dynamic_cast<ICMPv6TimeExceededMsg *>(icmpv6msg))
    {
        EV << "ICMPv6 Time Exceeded Message Received." << endl;
        errorOut(icmpv6msg);
    }
    else if (dynamic_cast<ICMPv6ParamProblemMsg *>(icmpv6msg))
    {
        EV << "ICMPv6 Parameter Problem Message Received." << endl;
        errorOut(icmpv6msg);
    }
    else if (dynamic_cast<ICMPv6EchoRequestMsg *>(icmpv6msg))
    {
        EV << "ICMPv6 Echo Request Message Received." << endl;
        processEchoRequest((ICMPv6EchoRequestMsg *)icmpv6msg);
    }
    else if (dynamic_cast<ICMPv6EchoReplyMsg *>(icmpv6msg))
    {
        EV << "ICMPv6 Echo Reply Message Received." << endl;
        processEchoReply((ICMPv6EchoReplyMsg *)icmpv6msg);
    }
    else
        error("Unknown message type received.\n");
}

void ICMPv6::processEchoRequest(ICMPv6EchoRequestMsg *request)
{
    //Create an ICMPv6 Reply Message
    ICMPv6EchoReplyMsg *reply = new ICMPv6EchoReplyMsg("Echo Reply");
    reply->setName((std::string(request->getName())+"-reply").c_str());
    reply->setType(ICMPv6_ECHO_REPLY);
    reply->encapsulate(request->decapsulate());

    // TBD check what to do if dest was multicast etc?
    IPv6ControlInfo *ctrl
        = check_and_cast<IPv6ControlInfo *>(request->getControlInfo());
    IPv6ControlInfo *replyCtrl = new IPv6ControlInfo();
    replyCtrl->setProtocol(IP_PROT_IPv6_ICMP);
    //set Msg's source addr as the dest addr of request msg.
    replyCtrl->setSrcAddr(ctrl->getDestAddr());
    //set Msg's dest addr as the source addr of request msg.
    replyCtrl->setDestAddr(ctrl->getSrcAddr());
    reply->setControlInfo(replyCtrl);

    delete request;
    sendToIP(reply);
}

void ICMPv6::processEchoReply(ICMPv6EchoReplyMsg *reply)
{
    IPv6ControlInfo *ctrl = check_and_cast<IPv6ControlInfo*>(reply->removeControlInfo());
    cPacket *payload = reply->decapsulate();
    payload->setControlInfo(ctrl);
    delete reply;
    send(payload, "pingOut");
}

void ICMPv6::sendEchoRequest(cPacket *msg)
{
    IPv6ControlInfo *ctrl = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());
    ctrl->setProtocol(IP_PROT_IPv6_ICMP);
    ICMPv6EchoRequestMsg *request = new ICMPv6EchoRequestMsg(msg->getName());
    request->setType(ICMPv6_ECHO_REQUEST);
    request->encapsulate(msg);
    request->setControlInfo(ctrl);
    sendToIP(request);
}

void ICMPv6::sendErrorMessage(IPv6Datagram *origDatagram, ICMPv6Type type, int code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // get ownership
    take(origDatagram);

    if (!validateDatagramPromptingError(origDatagram))
        return;

    ICMPv6Message *errorMsg;

    if (type == ICMPv6_DESTINATION_UNREACHABLE) errorMsg = createDestUnreachableMsg(code);
    //TODO: implement MTU support.
    else if (type == ICMPv6_PACKET_TOO_BIG) errorMsg = createPacketTooBigMsg(0);
    else if (type == ICMPv6_TIME_EXCEEDED) errorMsg = createTimeExceededMsg(code);
    else if (type == ICMPv6_PARAMETER_PROBLEM) errorMsg = createParamProblemMsg(code);
    else error("Unknown ICMPv6 error type\n");

    errorMsg->encapsulate(origDatagram);

    // debugging information
    EV << "sending ICMP error: (" << errorMsg->getClassName() << ")" << errorMsg->getName()
       << " type=" << type << " code=" << code << endl;

    // ICMP message length: the internet header plus the first 8 bytes of
    // the original datagram's data is returned to the sender
    //errorMessage->setByteLength(4 + origDatagram->getHeaderLength() + 8); FIXME What is this for?

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    if (origDatagram->getSrcAddress().isUnspecified())
    {
        // pretend it came from the IP layer
        IPv6ControlInfo *ctrlInfo = new IPv6ControlInfo();
        ctrlInfo->setSrcAddr(IPv6Address::LOOPBACK_ADDRESS); // FIXME maybe use configured loopback address
        ctrlInfo->setProtocol(IP_PROT_ICMP);
        errorMsg->setControlInfo(ctrlInfo);

        // then process it locally
        processICMPv6Message(errorMsg);
    }
    else
    {
        sendToIP(errorMsg, origDatagram->getSrcAddress());
    }
}

void ICMPv6::sendErrorMessage(cPacket *transportPacket, IPv6ControlInfo *ctrl, ICMPv6Type type, int code)
{
    Enter_Method("sendErrorMessage(transportPacket, ctrl, type=%d, code=%d)", type, code);

    IPv6Datagram *datagram = ctrl->removeOrigDatagram();
    datagram->encapsulate(transportPacket);
    sendErrorMessage(datagram, type, code);
}

void ICMPv6::sendToIP(ICMPv6Message *msg, const IPv6Address& dest)
{

    IPv6ControlInfo *ctrlInfo = new IPv6ControlInfo();
    ctrlInfo->setDestAddr(dest);
    ctrlInfo->setProtocol(IP_PROT_IPv6_ICMP);
    msg->setControlInfo(ctrlInfo);

    send(msg,"ipv6Out");
}

void ICMPv6::sendToIP(ICMPv6Message *msg)
{
    // assumes IPControlInfo is already attached
    send(msg,"ipv6Out");
}

ICMPv6Message *ICMPv6::createDestUnreachableMsg(int code)
{
    ICMPv6DestUnreachableMsg *errorMsg = new ICMPv6DestUnreachableMsg("Dest Unreachable");
    errorMsg->setType(ICMPv6_DESTINATION_UNREACHABLE);
    errorMsg->setCode(code);
    return errorMsg;
}

ICMPv6Message *ICMPv6::createPacketTooBigMsg(int mtu)
{
    ICMPv6PacketTooBigMsg *errorMsg = new ICMPv6PacketTooBigMsg("Packet Too Big");
    errorMsg->setType(ICMPv6_PACKET_TOO_BIG);
    errorMsg->setCode(0);//Set to 0 by sender and ignored by receiver.
    errorMsg->setMTU(mtu);
    return errorMsg;
}

ICMPv6Message *ICMPv6::createTimeExceededMsg(int code)
{
    ICMPv6TimeExceededMsg *errorMsg = new ICMPv6TimeExceededMsg("Time Exceeded");
    errorMsg->setType(ICMPv6_TIME_EXCEEDED);
    errorMsg->setCode(code);
    return errorMsg;
}

ICMPv6Message *ICMPv6::createParamProblemMsg(int code)
{
    ICMPv6ParamProblemMsg *errorMsg = new ICMPv6ParamProblemMsg("Parameter Problem");
    errorMsg->setType(ICMPv6_PARAMETER_PROBLEM);
    errorMsg->setCode(code);
    //TODO: What Pointer? section 3.4
    return errorMsg;
}

bool ICMPv6::validateDatagramPromptingError(IPv6Datagram *origDatagram)
{
    // don't send ICMP error messages for multicast messages
    if (origDatagram->getDestAddress().isMulticast())
    {
        EV << "won't send ICMP error messages for multicast message " << origDatagram << endl;
        delete origDatagram;
        return false;
    }

    // do not reply with error message to error message
    if (origDatagram->getTransportProtocol() == IP_PROT_IPv6_ICMP)
    {
        ICMPv6Message *recICMPMsg = check_and_cast<ICMPv6Message *>(origDatagram->getEncapsulatedPacket());
        if (recICMPMsg->getType()<128)
        {
            EV << "ICMP error received -- do not reply to it" << endl;
            delete origDatagram;
            return false;
        }
    }
    return true;
}

void ICMPv6::errorOut(ICMPv6Message *icmpv6msg)
{
    send(icmpv6msg, "errorOut");
}
