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
#include "UDPControlInfo_m.h"
#include "StringTokenizer.h"
#include "IPAddressResolver.h"


Define_Module(UDPSink);


void UDPSink::initialize()
{
    numReceived = 0;
    WATCH(numReceived);
}

void UDPSink::handleMessage(cMessage *msg)
{
    processPacket(msg);

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        displayString().setTagArg("t",0,buf);
    }

}

void UDPSink::printPacket(cMessage *msg)
{
    UDPControlInfo *controlInfo = check_and_cast<UDPControlInfo *>(msg->controlInfo());

    IPAddress src = controlInfo->getSrcAddr();
    IPAddress dest = controlInfo->getDestAddr();
    int sentPort = controlInfo->getSrcPort();
    int recPort = controlInfo->getDestPort();

    ev  << msg << endl;
    ev  << "Payload length: " << (msg->length()/8) << " bytes" << endl;
    ev  << "Src/Port: " << src << " :" << sentPort << "  ";
    ev  << "Dest/Port: " << dest << ":" << recPort << endl;
}

void UDPSink::processPacket(cMessage *msg)
{
    ev << "Received packet: ";
    printPacket(msg);
    delete msg;

    numReceived++;
}



//===============================================


Define_Module(UDPApp);

int UDPApp::counter;

void UDPApp::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage!=3)
        return;

    UDPSink::initialize();

    localPort = par("local_port");
    destPort = par("dest_port");
    msgLength = par("message_length");

    const char *destAddrs = par("dest_addresses");
    StringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
        destAddresses.push_back(IPAddressResolver().resolve(token));

    counter = 0;

    numSent = 0;
    WATCH(numSent);

    if (destAddresses.empty())
        return;

    cMessage *timer = new cMessage("sendTimer");
    scheduleAt((double)par("message_freq"), timer);
}

IPAddress UDPApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    return destAddresses[k];
}

void UDPApp::sendPacket()
{
    char msgName[32];
    sprintf(msgName,"udpAppData-%d", counter++);

    cMessage *payload = new cMessage(msgName);
    payload->setLength(msgLength);

    UDPControlInfo *controlInfo = new UDPControlInfo();
    IPAddress destAddr = chooseDestAddr();
    controlInfo->setDestAddr(destAddr);
    controlInfo->setSrcPort(localPort);
    controlInfo->setDestPort(destPort);
    payload->setControlInfo(controlInfo);

    ev << "Sending packet: ";
    printPacket(payload);

    send(payload, "to_udp");
    numSent++;
}

void UDPApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // send, then reschedule next sending
        sendPacket();
        scheduleAt(simTime()+(double)par("message_freq"), msg);
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


