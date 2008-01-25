/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#include <omnetpp.h>
#include <string.h>

#include "rsvp_message.h"
#include "RSVPInterface.h"
#include "IPControlInfo_m.h"

Define_Module(RSVPInterface);

void RSVPInterface::initialize()
{

}

void RSVPInterface::handleMessage(cMessage * msg)
{
    if (strcmp(msg->arrivalGate()->name(), "from_ip") == 0)
    {
        // received from IP layer
        processMsgFromIp(msg);
    }
    else
    {
        // received from application layer
        processMsgFromApp(msg);
    }
}



void RSVPInterface::processMsgFromIp(cMessage * msg)
{
/*
    delete msg->removeControlInfo();
*/
    send(msg, "to_rsvp_app");
/*
    // int i;
    // int applicationNo = -1;
    // int port;
    PathMessage *pMsg;
    ResvMessage *rMsg;
    PathTearMessage *ptMsg;
    PathErrorMessage *peMsg;
    // ResvTearMessage *rtMsg;
    // ResvErrorMessage *reMsg;

    cMessage *tpacket = msg; //FIXME was: TransportPacket *tpacket = check_and_cast<TransportPacket *>(msg);

    cMessage *rsvpMsg = (cMessage *) (tpacket->par("rsvp_data").objectValue());
    int msgKind = rsvpMsg->kind();

    int peerInf = 0;

    switch (msgKind)
    {
    case PATH_MESSAGE:

        pMsg = new PathMessage();
        pMsg->setContent(check_and_cast < PathMessage * >(rsvpMsg));

        if (rsvpMsg->hasPar("peerInf"))
        {
            peerInf = rsvpMsg->par("peerInf").longValue();
            pMsg->addPar("peerInf") = peerInf;
        }

        send(pMsg, "to_rsvp_app");

        break;

    case RESV_MESSAGE:
        rMsg = new ResvMessage();
        rMsg->setContent(check_and_cast < ResvMessage * >(rsvpMsg));
        send(rMsg, "to_rsvp_app");

        break;

    case PTEAR_MESSAGE:
        ptMsg = new PathTearMessage();
        ptMsg->setContent(check_and_cast < PathTearMessage * >(rsvpMsg));
        send(ptMsg, "to_rsvp_app");

        break;

    case PERROR_MESSAGE:
        peMsg = new PathErrorMessage();
        peMsg->setContent(check_and_cast < PathErrorMessage * >(rsvpMsg));
        send(peMsg, "to_rsvp_app");
        break;

    default:
        error("Unrecognised RSVP TE message types");
    }
*/
}

void RSVPInterface::processMsgFromApp(cMessage * msg)
{
    send(msg, "to_ip");

/*
    IPAddress src_addr = msg->hasPar("src_addr") ? msg->par("src_addr").stringValue() : IPAddress();
    IPAddress dest_addr = msg->hasPar("dest_addr") ? msg->par("dest_addr").stringValue() :  IPAddress();

    // attach control info to transport packet
    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(dest_addr);
    controlInfo->setSrcAddr(src_addr);
    controlInfo->setProtocol(IP_PROT_RSVP);
    msg->setControlInfo(controlInfo);

    send(msg, "to_ip");
*/

/*
    // FIXME was this: TransportPacket* tpacket = new TransportPacket(*msg);
    // FIXME then it was this: TransportPacket *tpacket = new TransportPacket(); *(cMessage *)tpacket = *msg;
    cMessage *tpacket = new cMessage(*msg);
    tpacket->addPar("rsvp_data") = msg; // FIXME ???


    // FIXME eliminate MY_ERROR_IP_ADDRESS? NULL and IPAddress.isNull() should be OK instead
    IPAddress src_addr =
        msg->hasPar("src_addr") ? msg->par("src_addr").stringValue() : MY_ERROR_IP_ADDRESS;
    IPAddress dest_addr =
        msg->hasPar("dest_addr") ? msg->par("dest_addr").stringValue() : MY_ERROR_IP_ADDRESS;

    // attach control info to transport packet
    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(dest_addr);
    if (!src_addr.equals(MY_ERROR_IP_ADDRESS))
        controlInfo->setSrcAddr(src_addr);
    controlInfo->setProtocol(IP_PROT_RSVP);

    tpacket->setControlInfo(controlInfo);

    send(tpacket, "to_ip");
*/
}
