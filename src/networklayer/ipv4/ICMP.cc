//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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

//  Cleanup and rewrite: Andras Varga, 2004

#include <omnetpp.h>
#include <string.h>

#include "IPDatagram.h"
#include "IPControlInfo.h"
#include "ICMP.h"

Define_Module(ICMP);


void ICMP::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();

    // process arriving ICMP message
    if (!strcmp(arrivalGate->getName(), "localIn"))
    {
        processICMPMessage(check_and_cast<ICMPMessage *>(msg));
        return;
    }

    // request from application
    if (!strcmp(arrivalGate->getName(), "pingIn"))
    {
        sendEchoRequest(PK(msg));
        return;
    }
}


void ICMP::sendErrorMessage(IPDatagram *origDatagram, ICMPType type, ICMPCode code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // get ownership
    take(origDatagram);

    // don't send ICMP error messages for multicast messages
    if (origDatagram->getDestAddress().isMulticast())
    {
        EV << "won't send ICMP error messages for multicast message " << origDatagram << endl;
        delete origDatagram;
        return;
    }

    // do not reply with error message to error message
    if (origDatagram->getTransportProtocol() == IP_PROT_ICMP)
    {
        ICMPMessage *recICMPMsg = check_and_cast<ICMPMessage *>(origDatagram->getEncapsulatedPacket());
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
    // the original datagram's data is returned to the sender.
    //
    // NOTE: since we just overwrite the errorMessage length without actually
    // truncating origDatagram, one can get "packet length became negative"
    // error when decapsulating the origDatagram on the receiver side.
    // A workaround is to avoid decapsulation, or to manually set the
    // errorMessage length to be larger than the encapsulated message.
    int dataLength = origDatagram->getByteLength() - origDatagram->getHeaderLength();
    int truncatedDataLength = dataLength <= 8 ? dataLength : 8;
    errorMessage->setByteLength(8 + origDatagram->getHeaderLength() + truncatedDataLength);

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    if (origDatagram->getSrcAddress().isUnspecified())
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
        sendToIP(errorMessage, origDatagram->getSrcAddress());
    }
}

void ICMP::sendErrorMessage(cPacket *transportPacket, IPControlInfo *ctrl, ICMPType type, ICMPCode code)
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
    reply->setName((std::string(request->getName())+"-reply").c_str());
    reply->setType(ICMP_ECHO_REPLY);

    // swap src and dest
    // TBD check what to do if dest was multicast etc?
    IPControlInfo *ctrl = check_and_cast<IPControlInfo *>(reply->getControlInfo());
    IPAddress src = ctrl->getSrcAddr();
    IPAddress dest = ctrl->getDestAddr();
    ctrl->setSrcAddr(dest);
    ctrl->setDestAddr(src);

    sendToIP(reply);
}

void ICMP::processEchoReply(ICMPMessage *reply)
{
    IPControlInfo *ctrl = check_and_cast<IPControlInfo*>(reply->removeControlInfo());
    cPacket *payload = reply->decapsulate();
    payload->setControlInfo(ctrl);
    delete reply;
    send(payload, "pingOut");
}

void ICMP::sendEchoRequest(cPacket *msg)
{
    IPControlInfo *ctrl = check_and_cast<IPControlInfo*>(msg->removeControlInfo());
    ctrl->setProtocol(IP_PROT_ICMP);
    ICMPMessage *request = new ICMPMessage(msg->getName());
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

