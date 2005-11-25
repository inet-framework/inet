//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <omnetpp.h>
#include "ICMPv6.h"


Define_Module(ICMPv6);


void ICMPv6::initialize()
{
    //...
}

void ICMPv6::handleMessage(cMessage *msg)
{
    // process arriving ICMP message
    if (msg->arrivalGate()->isName("fromIPv6"))
    {
        EV << "Processing ICMPv6 message.\n";
        processICMPv6Message(check_and_cast<ICMPv6Message *>(msg));
        return;
    }

    //TODO: need to refer to INET implementations and see how this works out.
    // request from application
    if (msg->arrivalGate()->isName("pingIn"))
    {
        //TODO: to be implemented
        //sendEchoRequest(msg);
        return;
    }
}

void ICMPv6::processICMPv6Message(ICMPv6Message *icmpv6msg)
{
    if (dynamic_cast<ICMPv6DestUnreachableMsg *>(icmpv6msg))
    {
        EV << "ICMPv6 Destination Unreachable Message Received." << endl;
        errorOut(icmpv6msg);
    }
    else if (dynamic_cast<ICMPv6PacketTooBigMsg *>(icmpv6msg))
    {
        //TODO: To be implemented
    }
    else if (dynamic_cast<ICMPv6TimeExceededMsg *>(icmpv6msg))
    {
        //TODO: To be implemented
    }
    else if (dynamic_cast<ICMPv6ParamProblemMsg *>(icmpv6msg))
    {
        //TODO: To be implemented
    }
    else if (dynamic_cast<ICMPv6EchoRequestMsg *>(icmpv6msg))
    {
        //TODO: To be implemented
    }
    else if (dynamic_cast<ICMPv6EchoReplyMsg *>(icmpv6msg))
    {
        //TODO: To be implemented
    }
    else
        error("Unknown message type received.\n");
}

void ICMPv6::sendErrorMessage(IPv6Datagram *origDatagram, ICMPv6Type type, int code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // get ownership
    take(origDatagram);

    if (!validateDatagramPromptingError(origDatagram))
        return;

    ICMPv6Message *errorMsg;

    // TODO finish! turn it into a switch(), etc
    if (type == ICMPv6_DESTINATION_UNREACHABLE) errorMsg = createDestUnreachableMsg(code);
    else if (type == 2) {}
    else if (type == 3) {}
    else if (type == 4) {}
    else error("Unknown ICMPv6 error type\n");

    errorMsg->encapsulate(origDatagram);

    // ICMP message length: the internet header plus the first 8 bytes of
    // the original datagram's data is returned to the sender
    //errorMessage->setByteLength(4 + origDatagram->headerLength() + 8); What is this for?

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    if (origDatagram->srcAddress().isUnspecified())
    {
        // pretend it came from the IP layer
        IPv6ControlInfo *ctrlInfo = new IPv6ControlInfo();
        ctrlInfo->setSrcAddr(IPv6Address::LOOPBACK_ADDRESS); // FIXME maybe use configured loopback address
        ctrlInfo->setProtocol(IP_PROT_ICMP);
        errorMsg->setControlInfo(ctrlInfo);

        // then process it locally
        handleMessage(errorMsg);
    }
    else
    {
        sendToIP(errorMsg, origDatagram->srcAddress());
    }

    // debugging information
    //ev << "sending ICMP error: " << errorMsg->type() << " / " << errorMsg->code() << endl;
}

void ICMPv6::sendToIP(ICMPv6Message *msg, const IPv6Address& dest)
{

    IPv6ControlInfo *ctrlInfo = new IPv6ControlInfo();
    ctrlInfo->setDestAddr(dest);
    ctrlInfo->setProtocol(IP_PROT_IPv6_ICMP);
    msg->setControlInfo(ctrlInfo);

    send(msg,"toIPv6");
}

ICMPv6Message *ICMPv6::createDestUnreachableMsg(int code)
{
    ICMPv6DestUnreachableMsg *errorMsg
        = new ICMPv6DestUnreachableMsg("Dest Unreachable");
    errorMsg->setType(ICMPv6_DESTINATION_UNREACHABLE);
    errorMsg->setCode(code);
    return errorMsg;
}

ICMPv6Message *ICMPv6::createPacketTooBigMsg(int mtu)
{
    //TODO: Not implemented yet
    return &ICMPv6Message();
}

ICMPv6Message *ICMPv6::createTimeExceededMsg(int code)
{
    //TODO: Not implemented yet
    return &ICMPv6Message();
}

ICMPv6Message *createParamProblemMsg(int code)
{
    //TODO: Not implemented yet
    return &ICMPv6Message();
}

bool ICMPv6::validateDatagramPromptingError(IPv6Datagram *origDatagram)
{
    // don't send ICMP error messages for multicast messages
    if (origDatagram->destAddress().isMulticast())
    {
        EV << "won't send ICMP error messages for multicast message " << origDatagram << endl;
        delete origDatagram;
        return false;
    }

    // do not reply with error message to error message
    if (origDatagram->transportProtocol() == IP_PROT_IPv6_ICMP)
    {
        ICMPv6Message *recICMPMsg = check_and_cast<ICMPv6Message *>(origDatagram->encapsulatedMsg());
        if (recICMPMsg->type()<128)
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
