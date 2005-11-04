//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
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


//
// Author: Jochen Reber
// Rewrite: Andras Varga 2004,2005
//

#include <omnetpp.h>
#include <string.h>
#include "UDPPacket.h"
#include "UDP.h"
#include "IPControlInfo_m.h"
#include "IPv6ControlInfo_m.h"


Define_Module( UDP );

void UDP::initialize()
{
    // don't dispatch to apps (send everything to gate 0) if: only 1 app + no port bound
    dispatchByPort = (gateSize("from_app")!=1);

    WATCH_MAP(port2indexMap);

    numSent = 0;
    numPassedUp = 0;
    numDroppedWrongPort = 0;
    numDroppedBadChecksum = 0;
    WATCH(numSent);
    WATCH(numPassedUp);
    WATCH(numDroppedWrongPort);
    WATCH(numDroppedBadChecksum);
}

void UDP::handleMessage(cMessage *msg)
{
    // received from IP layer
    if (msg->arrivedOn("from_ip") || msg->arrivedOn("from_ipv6"))
    {
        processMsgFromIP(check_and_cast<UDPPacket *>(msg));
    }
    else // received from application layer
    {
        if (msg->kind()==UDP_C_DATA)
            processMsgFromApp(msg);
        else
            processCommandFromApp(msg);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void UDP::bind(int gateIndex, int udpPort)
{
    dispatchByPort = true;
    if (port2indexMap.find(udpPort)!=port2indexMap.end())
        error("bind(): port %d already bound, to app on gate index %d", udpPort, port2indexMap[udpPort]);
    port2indexMap[udpPort] = gateIndex;
}

void UDP::unbind(int gateIndex, int udpPort)
{
    IntMap::iterator it = port2indexMap.find(udpPort);
    if (it!=port2indexMap.end())
        port2indexMap.erase(it);
}

int UDP::findAppGateForPort(int destPort)
{
    if (!dispatchByPort)
        return 0;

    IntMap::iterator it = port2indexMap.find(destPort);
    return it==port2indexMap.end() ? -1 : it->second;
}

void UDP::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %d pks\nsent: %d pks", numPassedUp, numSent);
    if (numDroppedWrongPort>0)
    {
        sprintf(buf+strlen(buf), "\ndropped (no app): %d pks", numDroppedWrongPort);
        displayString().setTagArg("i",1,"red");
    }
    displayString().setTagArg("t",0,buf);
}

void UDP::processMsgFromIP(UDPPacket *udpPacket)
{
    // simulate checksum: discard packet if it has bit error
    if (udpPacket->hasBitError())
    {
        delete udpPacket;
        numDroppedBadChecksum++;
        return;
    }

    // look up app gate
    int destPort = udpPacket->destinationPort();
    int appGateIndex = findAppGateForPort(destPort);
    if (appGateIndex == -1)
    {
        delete udpPacket;
        numDroppedWrongPort++;
        return;
    }

    // get src/dest addresses
    IPvXAddress srcAddr, destAddr;
    int inputPort;
    if (dynamic_cast<IPControlInfo *>(udpPacket->controlInfo())!=NULL)
    {
        IPControlInfo *controlInfo = (IPControlInfo *)udpPacket->removeControlInfo();
        srcAddr = controlInfo->srcAddr();
        destAddr = controlInfo->destAddr();
        inputPort = controlInfo->inputPort();
        delete controlInfo;
    }
    else if (dynamic_cast<IPv6ControlInfo *>(udpPacket->controlInfo())!=NULL)
    {
        IPv6ControlInfo *controlInfo = (IPv6ControlInfo *)udpPacket->removeControlInfo();
        srcAddr = controlInfo->srcAddr();
        destAddr = controlInfo->destAddr();
        inputPort = controlInfo->inputGateIndex();
        delete controlInfo;
    }
    else
    {
        error("(%s)%s arrived from lower layer without control info", udpPacket->className(), udpPacket->name());
    }

    // send payload with UDPControlInfo up to the application
    UDPControlInfo *udpControlInfo = new UDPControlInfo();
    udpControlInfo->setSrcAddr(srcAddr);
    udpControlInfo->setDestAddr(destAddr);
    udpControlInfo->setSrcPort(udpPacket->sourcePort());
    udpControlInfo->setDestPort(udpPacket->destinationPort());
    udpControlInfo->setInputPort(inputPort);

    cMessage *payload = udpPacket->decapsulate();
    payload->setControlInfo(udpControlInfo);
    delete udpPacket;

    send(payload, "to_app", appGateIndex);
    numPassedUp++;
}

void UDP::processMsgFromApp(cMessage *appData)
{
    UDPControlInfo *udpControlInfo = check_and_cast<UDPControlInfo *>(appData->removeControlInfo());

    UDPPacket *udpPacket = new UDPPacket(appData->name());
    udpPacket->setByteLength(UDP_HEADER_BYTES);
    udpPacket->encapsulate(appData);

    // set source and destination port
    udpPacket->setSourcePort(udpControlInfo->getSrcPort());
    udpPacket->setDestinationPort(udpControlInfo->getDestPort());

    if (!udpControlInfo->getDestAddr().isIPv6())
    {
        // send to IPv4
        IPControlInfo *ipControlInfo = new IPControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(udpControlInfo->getSrcAddr().get4());
        ipControlInfo->setDestAddr(udpControlInfo->getDestAddr().get4());
        ipControlInfo->setOutputPort(udpControlInfo->getOutputPort());
        udpPacket->setControlInfo(ipControlInfo);
        delete udpControlInfo;

        send(udpPacket,"to_ip");
    }
    else
    {
        // send to IPv6
        IPv6ControlInfo *ipControlInfo = new IPv6ControlInfo();
        ipControlInfo->setProtocol(IP_PROT_UDP);
        ipControlInfo->setSrcAddr(udpControlInfo->getSrcAddr().get6());
        ipControlInfo->setDestAddr(udpControlInfo->getDestAddr().get6());
        udpPacket->setControlInfo(ipControlInfo);
        delete udpControlInfo;

        send(udpPacket,"to_ipv6");
    }
    numSent++;
}

void UDP::processCommandFromApp(cMessage *msg)
{
    UDPControlInfo *udpControlInfo = check_and_cast<UDPControlInfo *>(msg->removeControlInfo());
    switch (msg->kind())
    {
        case UDP_C_BIND:
            bind(msg->arrivalGate()->index(), udpControlInfo->getSrcPort());
            break;
        case UDP_C_UNBIND:
            unbind(msg->arrivalGate()->index(), udpControlInfo->getSrcPort());
            break;
        default:
            error("unknown command code (message kind) %d received from app", msg->kind());
    }

    delete udpControlInfo;
    delete msg;
}


