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


// No module definition required, since there's no NED definition
// Define_Module( UDPAppBase );

void UDPAppBase::initialize()
{
    nodeName = par("nodename");
    localPort = par("local_port");
    destPort = par("dest_port");
    msgLength = par("message_length");
    msgFreq = par("message_freq");
    destType = par("routeDestNo");
}


//=====================================================

Define_Module(UDPServer);

void UDPServer::handleMessage(cMessage *msg)
{
    UDPInterfacePacket *udpIfPacket = check_and_cast<UDPInterfacePacket *>(msg);
    cMessage *payload = udpIfPacket->decapsulate();

    IPAddress src = udpIfPacket->getSrcAddr();
    IPAddress dest = udpIfPacket->getDestAddr();
    int sentPort = udpIfPacket->getSrcPort();
    int recPort = udpIfPacket->getDestPort();

    ev  << nodeName.c_str() << " UDP Server: Packet received: " << payload << endl;
    ev  << "Payload length: " << (payload->length()/8) << " bytes" << endl;
    ev  << "Src/Port: " << src.getString() << " / " << sentPort << "  ";
    ev  << "Dest/Port: " << dest.getString() << " / " << recPort << endl;

    delete udpIfPacket;
    delete payload;
}


//===============================================


Define_Module(UDPClient);

void UDPClient::initialize()
{
    UDPAppBase::initialize();
    contCtr++;
}

void UDPClient::activity()
{
    int contCtr = intrand(100);

    wait(1);

    while(true)
    {
        wait(truncnormal(msgFreq, msgFreq * 0.1));

        cMessage *payload = new cMessage();
        payload->setLength(8*(1+intrand(msgLength)));

        UDPInterfacePacket *udpIfPacket = new UDPInterfacePacket();
        udpIfPacket->encapsulate(payload);

        IPAddress destAddr;
        chooseDestAddr(destAddr);
        udpIfPacket->setDestAddr(destAddr);
        udpIfPacket->setSrcPort(localPort);
        udpIfPacket->setDestPort(destPort);

        ev << nodeName.c_str() <<" UDP App: Packet sent: " << payload << endl;
        ev << "Payload length: " << (payload->length()/8) << " bytes" << endl;
        ev << "Src/Port: unknown / " << localPort << "  ";
        ev << "Dest/Port: " << destAddr.getString() << " / " << destPort << endl;

        send(udpIfPacket, "to_udp");
    }
}


void UDPClient::chooseDestAddr(IPAddress& dest)
{
    // data based on test2.irt
    int typeCtr = 3;
    int destCtr[] = {6, 5, 12};
    char *destAddrArray[][20] = {
        // MC Network 1
        { "225.0.0.1", "230.0.0.1", "230.0.1.0", "230.0.1.1", "10.0.1.3", "127.0.0.1"},
        // TCP/UDP complete netowrk
        { "10.0.0.1", "10.0.0.2", "10.0.0.3", "10.0.0.4", "127.0.0.1"},
        // MC Network 2
        { "172.0.0.1", "172.0.0.2", "172.0.0.3", "172.0.1.1", "172.0.2.1", "172.0.2.2",
          "225.0.0.1", "225.0.0.2", "225.0.0.3", "225.0.1.1", "225.0.1.2", "225.0.2.1"}
    };

    if (destType >= typeCtr)
        opp_error("wrong routeDestNo value %d", destType);

    dest = destAddrArray[destType][intrand(destCtr[destType])];
}

