//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 Andras Varga
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
    processPacket(check_and_cast<cPacket *>(msg));

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        getDisplayString().setTagArg("t",0,buf);
    }

}

void IPTrafSink::printPacket(cPacket *msg)
{
    IPvXAddress src, dest;
    int protocol = -1;
    if (dynamic_cast<IPControlInfo *>(msg->getControlInfo())!=NULL)
    {
        IPControlInfo *ctrl = (IPControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }
    else if (dynamic_cast<IPv6ControlInfo *>(msg->getControlInfo())!=NULL)
    {
        IPv6ControlInfo *ctrl = (IPv6ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }

    ev  << msg << endl;
    ev  << "Payload length: " << msg->getByteLength() << " bytes" << endl;
    if (protocol!=-1)
        ev  << "src: " << src << "  dest: " << dest << "  protocol=" << protocol << "\n";
}

void IPTrafSink::processPacket(cPacket *msg)
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

    if (numPackets > 0) {
        cMessage *timer = new cMessage("sendTimer");
        scheduleAt(startTime, timer);
    }
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

    cPacket *payload = new cPacket(msgName);
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

        send(payload, "ipOut");
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

        send(payload, "ipv6Out");
    }
    numSent++;
}

void IPTrafGen::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // send, then reschedule next sending
        sendPacket();

        if (!numPackets || numSent<numPackets)
            scheduleAt(simTime()+(double)par("packetInterval"), msg);
        else
            delete msg;
    }
    else
    {
        // process incoming packet
        processPacket(PK(msg));
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t",0,buf);
    }
}


