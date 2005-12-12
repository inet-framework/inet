//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 Andras Varga
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
#include "IPTrafGen.h"
#include "IPControlInfo.h"
#include "IPv6ControlInfo.h"
#include "IPAddressResolver.h"


Define_Module(IPTrafSink);


void IPTrafSink::initialize()
{
    numReceived = 0;
    WATCH(numReceived);
}

void IPTrafSink::handleMessage(cMessage *msg)
{
    processPacket(msg);

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        displayString().setTagArg("t",0,buf);
    }

}

void IPTrafSink::printPacket(cMessage *msg)
{
    IPvXAddress src, dest;
    int protocol = -1;
    if (dynamic_cast<IPControlInfo *>(msg->controlInfo())!=NULL)
    {
        IPControlInfo *ctrl = (IPControlInfo *)msg->controlInfo();
        src = ctrl->srcAddr();
        dest = ctrl->destAddr();
        protocol = ctrl->protocol();
    }
    else if (dynamic_cast<IPv6ControlInfo *>(msg->controlInfo())!=NULL)
    {
        IPv6ControlInfo *ctrl = (IPv6ControlInfo *)msg->controlInfo();
        src = ctrl->srcAddr();
        dest = ctrl->destAddr();
        protocol = ctrl->protocol();
    }

    ev  << msg << endl;
    ev  << "Payload length: " << msg->byteLength() << " bytes" << endl;
    if (protocol!=-1)
        ev  << "src: " << src << "  dest: " << dest << "  protocol=" << protocol << "\n";
}

void IPTrafSink::processPacket(cMessage *msg)
{
    EV << "Received packet: ";
    printPacket(msg);
    delete msg;

    numReceived++;
}



//===============================================


Define_Module(IPTrafGen);

int IPTrafGen::counter;

void IPTrafGen::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage!=3)
        return;

    IPTrafSink::initialize();

    protocol = par("protocol");
    msgByteLength = par("packetLength");
    numPackets = par("numPackets");
    simtime_t startTime = par("startTime");

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
        destAddresses.push_back(IPAddressResolver().resolve(token));

    counter = 0;

    numSent = 0;
    WATCH(numSent);

    if (destAddresses.empty())
        return;

    cMessage *timer = new cMessage("sendTimer");
    scheduleAt(startTime, timer);
}

IPvXAddress IPTrafGen::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    return destAddresses[k];
}

void IPTrafGen::sendPacket()
{
    char msgName[32];
    sprintf(msgName,"appData-%d", counter++);

    cMessage *payload = new cMessage(msgName);
    payload->setByteLength(msgByteLength);

    IPvXAddress destAddr = chooseDestAddr();
    if (!destAddr.isIPv6())
    {
        // send to IPv4
        IPControlInfo *controlInfo = new IPControlInfo();
        controlInfo->setDestAddr(destAddr.get4());
        controlInfo->setProtocol(protocol);
        payload->setControlInfo(controlInfo);

        EV << "Sending packet: ";
        printPacket(payload);

        send(payload, "to_ip");
    }
    else
    {
        // send to IPv6
        IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
        controlInfo->setDestAddr(destAddr.get6());
        controlInfo->setProtocol(protocol);
        payload->setControlInfo(controlInfo);

        EV << "Sending packet: ";
        printPacket(payload);

        send(payload, "to_ipv6");
    }
    numSent++;
}

void IPTrafGen::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // send, then reschedule next sending
        sendPacket();

        if (numSent<numPackets)
            scheduleAt(simTime()+(double)par("packetInterval"), msg);
    }
    else
    {
        // process incoming packet
        processPacket(msg);
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        displayString().setTagArg("t",0,buf);
    }
}


