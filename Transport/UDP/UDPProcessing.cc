//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
// author: Jochen Reber
//

#include <omnetpp.h>
#include <string.h>

#include "UDPPacket.h"
#include "UDPProcessing.h"
#include "IPInterfacePacket.h"

Define_Module( UDPProcessing );

void UDPProcessing::initialize()
{
    applTable.size = gateSize("to_application");
    applTable.port = new int[applTable.size];  // FIXME free it in dtor (or change to std::map)

    // if there's only one app and without "local_port" parameter, don't dispatch
    // incoming packets by port number.
    if (applTable.size==1 && gate("to_application",0)->toGate() &&
        gate("to_application",0)->toGate()->ownerModule()->hasPar("local_port")==false)
    {
        dispatchByPort = false;
    }
    else
    {
        dispatchByPort = true;
        for (int i=0; i<applTable.size; i++)
        {
            // "local_port" parameter of connected app module
            cGate *appgate = gate("to_application",i)->toGate();
            applTable.port[i] = appgate ? appgate->ownerModule()->par("local_port") : -1;
        }
    }

    WATCH(dispatchByPort);

    numSent = 0;
    numPassedUp = 0;
    numDroppedWrongPort = 0;
    numDroppedBadChecksum = 0;
    WATCH(numSent);
    WATCH(numPassedUp);
    WATCH(numDroppedWrongPort);
    WATCH(numDroppedBadChecksum);
}

void UDPProcessing::handleMessage(cMessage *msg)
{
    // received from IP layer
    if (!strcmp(msg->arrivalGate()->name(), "from_ip"))
    {
        processMsgFromIp(check_and_cast<IPInterfacePacket *>(msg));
    }
    else // received from application layer
    {
        processMsgFromApp(check_and_cast<UDPInterfacePacket *>(msg));
    }
}


void UDPProcessing::processMsgFromIp(IPInterfacePacket *ipIfPacket)
{
    // decapsulate UDPPacket from IP
    UDPPacket *udpPacket = check_and_cast<UDPPacket *>(ipIfPacket->decapsulate());

    // errorcheck, if applicable
    if (!udpPacket->checksumValid())
    {
        // throw packet away if bit error discovered
        // assume checksum found biterror
        delete ipIfPacket;
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
        delete ipIfPacket;
        numDroppedWrongPort++;
        return;
    }

    // wrap payload into UDPInterfacePacket and pass up to application
    UDPInterfacePacket *udpIfPacket = new UDPInterfacePacket();

    cMessage *payload = udpPacket->decapsulate();
    if (payload)
        udpIfPacket->encapsulate(payload);

    udpIfPacket->setSrcAddr(ipIfPacket->srcAddr());
    udpIfPacket->setDestAddr(ipIfPacket->destAddr());
    udpIfPacket->setCodePoint(ipIfPacket->diffServCodePoint());
    udpIfPacket->setSrcPort(udpPacket->sourcePort());
    udpIfPacket->setDestPort(udpPacket->destinationPort());
    //udpIfPacket->setKind(udpPacket->msgKind()); // FIXME why ???
    delete ipIfPacket;
    delete udpPacket;

    send(udpIfPacket, "to_application", appGateIndex);
    numPassedUp++;
}

void UDPProcessing::processMsgFromApp(UDPInterfacePacket *udpIfPacket)
{
    UDPPacket *udpPacket = new UDPPacket();
    udpPacket->setLength(8*UDP_HEADER_BYTES);
    //udpPacket->setMsgKind(udpIfPacket->kind()); --FIXME why???

    cMessage *payload = udpIfPacket->decapsulate();
    if (payload)
        udpPacket->encapsulate(payload);

    // set source and destination port
    udpPacket->setSourcePort(udpIfPacket->getSrcPort());
    udpPacket->setDestinationPort(udpIfPacket->getDestPort());

    IPInterfacePacket *ipIfPacket = new IPInterfacePacket();
    ipIfPacket->encapsulate(udpPacket);
    ipIfPacket->setProtocol(IP_PROT_UDP);
    ipIfPacket->setSrcAddr(udpIfPacket->getSrcAddr());
    ipIfPacket->setDestAddr(udpIfPacket->getDestAddr());
    delete udpIfPacket;

    // send to IP
    send(ipIfPacket,"to_ip");
    numSent++;
}

int UDPProcessing::findAppGateForPort(int destPort)
{
    if (!dispatchByPort)
        return 0;

    for (int i=0; i<applTable.size; i++)
        if (applTable.port[i]==destPort)
            return i;
    return -1;
}

