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


#include <omnetpp.h>
#include "UDPApp.h"
#include "UDPInterfacePacket_m.h"
#include "StringTokenizer.h"


Define_Module(UDPServerApp);


void UDPServerApp::initialize()
{
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);
}

void UDPServerApp::handleMessage(cMessage *msg)
{
    UDPInterfacePacket *udpIfPacket = check_and_cast<UDPInterfacePacket *>(msg);
    cMessage *payload = udpIfPacket->decapsulate();

    IPAddress src = udpIfPacket->getSrcAddr();
    IPAddress dest = udpIfPacket->getDestAddr();
    int sentPort = udpIfPacket->getSrcPort();
    int recPort = udpIfPacket->getDestPort();

    ev  << "Packet received: " << payload << endl;
    ev  << "Payload length: " << (payload->length()/8) << " bytes" << endl;
    ev  << "Src/Port: " << src << " / " << sentPort << "  ";
    ev  << "Dest/Port: " << dest << " / " << recPort << endl;

    numReceived++;

    delete udpIfPacket;
    delete payload;
}


//===============================================


Define_Module(UDPClientApp);

void UDPClientApp::initialize()
{
    localPort = par("local_port");
    destPort = par("dest_port");
    msgLength = par("message_length");

    const char *destAddrs = par("dest_addresses");
    StringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
        destAddresses.push_back(IPAddress(token));

    counter = 0;

    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);

    cMessage *timer = new cMessage("sendTimer");
    scheduleAt((double)par("message_freq"), timer);
}


void UDPClientApp::handleMessage(cMessage *msg)
{
    // reschedule next sending
    scheduleAt(simTime()+(double)par("message_freq"), msg);

    char msgName[32];
    sprintf(msgName,"udpAppData-%d", counter++);

    cMessage *payload = new cMessage(msgName);
    payload->setLength(msgLength);

    UDPInterfacePacket *udpIfPacket = new UDPInterfacePacket();
    udpIfPacket->encapsulate(payload);

    IPAddress destAddr = chooseDestAddr();
    udpIfPacket->setDestAddr(destAddr);
    udpIfPacket->setSrcPort(localPort);
    udpIfPacket->setDestPort(destPort);

    ev << "Packet sent: " << payload << endl;
    ev << "Payload length: " << (payload->length()/8) << " bytes" << endl;
    ev << "Src/Port: unknown / " << localPort << "  ";
    ev << "Dest/Port: " << destAddr << " / " << destPort << endl;

    send(udpIfPacket, "to_udp");
    numSent++;
}


IPAddress UDPClientApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    return destAddresses[k];
}

