//
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

/*
    file: UDPProcessing.cc
    Purpose: Implementation of UDP protocoll
    Initialisation:
        error check validity initialized as parameter
    Responsibilities:
        initialize with port-to-application-table
            read from NED-file

        receive message with encoded ports and IP addresses as parameters
            from application:
        create UDPPacket from it.
        encapsulate it into IPInterfacePacket
        pass it on to the IP layer

        receive IPInterfacePacket from IP Layer:
        decapsulate UDPPacket
        check biterror if checksum enabled
            -> throw away if biterror found
        map port to correct application-gate
        pass UDPPacket to the selected gate

        claim Kernel at the beginning of processing,
            release Kernel at the end

    author: Jochen Reber
*/

#include <omnetpp.h>
#include <string.h>

#include "UDPPacket.h"
#include "UDPProcessing.h"
#include "IPInterfacePacket.h"

Define_Module( UDPProcessing );

void UDPProcessing::initialize()
{
    applTable.size = gateSize("to_application");

    for (int i=0; i<applTable.size; i++)
    {
        // FIXME what the hell?????????????????
        applTable.port[i] = gate("to_application",i)->toGate()->ownerModule()->par("local_port");
    }

}

void UDPProcessing::handleMessage(cMessage *msg)
{
    // received from IP layer
    if (!strcmp(msg->arrivalGate()->name(), "from_ip"))
    {
        processMsgFromIp(msg);
    }
    else // received from application layer
    {
        processMsgFromApp(msg);
    }
}


void UDPProcessing::processMsgFromIp(cMessage *msg)
{
    int i;
    int applicationNo = -1;
    int port;
    // udpMessage is to be sent to application

    IPInterfacePacket *iPacket = (IPInterfacePacket *)msg;
    UDPPacket *udpPacket = (UDPPacket *) iPacket->decapsulate();
    cMessage *udpMessage = new cMessage(*((cMessage *)udpPacket)); // FIXME what's this?????????????

    // errorcheck, if applicable
    if (udpPacket->getChecksumValid())
    {
        // throw packet away if bit error discovered
        // assume checksum found biterror
        if (udpPacket->hasBitError())
        {
            delete iPacket;
            delete udpPacket;
            delete udpMessage;
            return;
        }
    }

    // check the names of the udpMessage parameters
    // iffy assignment, please check

    // FIXME should be some UDPInterfacePacket or something
    udpMessage->addPar("src_addr").setStringValue(iPacket->srcAddr());
    udpMessage->addPar("dest_addr").setStringValue(iPacket->destAddr());

    udpMessage->addPar("codepoint") = iPacket->diffServCodePoint();
    udpMessage->addPar("src_port") = udpPacket->sourcePort();
    port = udpMessage->addPar("dest_port") = udpPacket->destinationPort();
    udpMessage->setKind(udpPacket->msgKind());
    udpMessage->setLength(udpPacket->length());

    delete iPacket;
    delete udpPacket;

    // lookup end gate
    for (i=0; i < applTable.size; i++)
    {
        if (port == applTable.port[i])
        {
            applicationNo = i;
        }
    }

    // send only if gate to application is found
    if (applicationNo != -1)
    {
        send(udpMessage, "to_application", applicationNo);
    } else
    {
        delete udpMessage;
    }
}

void UDPProcessing::processMsgFromApp(cMessage *msg)
{
    // FIXME *msg NEEDS to be included in constructor. Correct later
    UDPPacket *udpPacket = new UDPPacket();

    udpPacket->setLength(msg->length());
    udpPacket->setMsgKind(msg->kind());

    // set source and destination port
    udpPacket->setSourcePort(msg->hasPar("src_port") ? (int)msg->par("src_port") : 255);
    udpPacket->setDestinationPort(msg->hasPar("dest_port") ? (int)msg->par("dest_port") : 255);

    IPInterfacePacket *iPacket = new IPInterfacePacket();
    iPacket->encapsulate(udpPacket);
    iPacket->setProtocol(IP_PROT_UDP);

    if (msg->hasPar("src_addr"))
    {
        IPAddress src_addr =  msg->par("src_addr").stringValue();
        iPacket->setSrcAddr(src_addr);
    }

    if (msg->hasPar("dest_addr"))
    {
        IPAddress dest_addr =  msg->par("dest_addr").stringValue();
        iPacket->setDestAddr(dest_addr);
    }
    else
    {
        opp_error("processMsgFromApp(): no dest_addr in message");
    }

    // we don't set other values now

    // send to IP
    send(iPacket,"to_ip");

    delete msg;

}


