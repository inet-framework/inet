//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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

//  Cleanup and rewrite: Andras Varga, 2004

#include <omnetpp.h>
#include <string.h>

#include "IPDatagram.h"
#include "IPControlInfo.h"
#include "ICMP.h"

Define_Module(ICMP);


void ICMP::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->arrivalGate();

    // process arriving ICMP message
    if (!strcmp(arrivalGate->name(), "localIn"))
    {
        processICMPMessage(check_and_cast<ICMPMessage *>(msg));
        return;
    }

    // request from application
    if (!strcmp(arrivalGate->name(), "pingIn"))
    {
        sendEchoRequest(msg);
        return;
    }
}


void ICMP::sendErrorMessage(IPDatagram *origDatagram, ICMPType type, ICMPCode code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // get ownership
    take(origDatagram);

    // don't send ICMP error messages for multicast messages
    if (origDatagram->destAddress().isMulticast())
    {
        EV << "won't send ICMP error messages for multicast message " << origDatagram << endl;
        delete origDatagram;
        return;
    }

    // do not reply with error message to error message
    if (origDatagram->transportProtocol() == IP_PROT_ICMP)
    {
        ICMPMessage *recICMPMsg = check_and_cast<ICMPMessage *>(origDatagram->encapsulatedMsg());
        if (recICMPMsg->getType()<128)
        {
            EV << "ICMP error received -- do not reply to it" << endl;
            delete origDatagram;
            return;
        }
    }

    // assemble a message name
    char msgname[32];
    static long ctr;
    sprintf(msgname, "ICMP-error-#%ld-type%d-code%d", ++ctr, type, code);

    // debugging information
    EV << "sending ICMP error " << msgname << endl;

    // create and send ICMP packet
    ICMPMessage *errorMessage = new ICMPMessage(msgname);
    errorMessage->setType(type);
    errorMessage->setCode(code);
    errorMessage->encapsulate(origDatagram);
    // ICMP message length: the internet header plus the first 8 bytes of
    // the original datagram's data is returned to the sender
    errorMessage->setByteLength(8 + origDatagram->headerLength() + 8);

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    if (origDatagram->srcAddress().isUnspecified())
    {
        // pretend it came from the IP layer
        IPControlInfo *controlInfo = new IPControlInfo();
        controlInfo->setSrcAddr(IPAddress::LOOPBACK_ADDRESS); // FIXME maybe use configured loopback address
        controlInfo->setProtocol(IP_PROT_ICMP);
        errorMessage->setControlInfo(controlInfo);

        // then process it locally
        processICMPMessage(errorMessage);
    }
    else
    {
        sendToIP(errorMessage, origDatagram->srcAddress());
    }
}

void ICMP::sendErrorMessage(cMessage *transportPacket, IPControlInfo *ctrl, ICMPType type, ICMPCode code)
{
    Enter_Method("sendErrorMessage(transportPacket, ctrl, type=%d, code=%d)", type, code);

    IPDatagram *datagram = ctrl->removeOrigDatagram();
    take(transportPacket);
    take(datagram);
    datagram->encapsulate(transportPacket);
    sendErrorMessage(datagram, type, code);
}

void ICMP::processICMPMessage(ICMPMessage *icmpmsg)
{
    switch (icmpmsg->getType())
    {
        case ICMP_DESTINATION_UNREACHABLE:
            errorOut(icmpmsg);
            break;
        case ICMP_REDIRECT:
            errorOut(icmpmsg);
            break;
        case ICMP_TIME_EXCEEDED:
            errorOut(icmpmsg);
            break;
        case ICMP_PARAMETER_PROBLEM:
            errorOut(icmpmsg);
            break;
        case ICMP_ECHO_REQUEST:
            processEchoRequest(icmpmsg);
            break;
        case ICMP_ECHO_REPLY:
            processEchoReply(icmpmsg);
            break;
        case ICMP_TIMESTAMP_REQUEST:
            processEchoRequest(icmpmsg);
            break;
        case ICMP_TIMESTAMP_REPLY:
            processEchoReply(icmpmsg);
            break;
        default:
            opp_error("Unknown ICMP type %d", icmpmsg->getType());
    }
}

void ICMP::errorOut(ICMPMessage *icmpmsg)
{
    send(icmpmsg, "errorOut");
}

void ICMP::processEchoRequest(ICMPMessage *request)
{
    // turn request into a reply
    ICMPMessage *reply = request;
    reply->setName((std::string(request->name())+"-reply").c_str());
    reply->setType(ICMP_ECHO_REPLY);

    // swap src and dest
    // TBD check what to do if dest was multicast etc?
    IPControlInfo *ctrl = check_and_cast<IPControlInfo *>(reply->controlInfo());
    IPAddress src = ctrl->srcAddr();
    IPAddress dest = ctrl->destAddr();
    ctrl->setSrcAddr(dest);
    ctrl->setDestAddr(src);

    sendToIP(reply);
}

void ICMP::processEchoReply(ICMPMessage *reply)
{
    IPControlInfo *ctrl = check_and_cast<IPControlInfo*>(reply->removeControlInfo());
    cMessage *payload = reply->decapsulate();
    payload->setControlInfo(ctrl);
    delete reply;
    send(payload, "pingOut");
}

void ICMP::sendEchoRequest(cMessage *msg)
{
    IPControlInfo *ctrl = check_and_cast<IPControlInfo*>(msg->removeControlInfo());
    ctrl->setProtocol(IP_PROT_ICMP);
    ICMPMessage *request = new ICMPMessage(msg->name());
    request->setType(ICMP_ECHO_REQUEST);
    request->encapsulate(msg);
    request->setControlInfo(ctrl);
    sendToIP(request);
}

void ICMP::sendToIP(ICMPMessage *msg, const IPAddress& dest)
{
    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(dest);
    controlInfo->setProtocol(IP_PROT_ICMP);
    msg->setControlInfo(controlInfo);

    send(msg, "sendOut");
}

void ICMP::sendToIP(ICMPMessage *msg)
{
    // assumes IPControlInfo is already attached
    send(msg, "sendOut");
}

