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


#include "inet/common/INETDefs.h"

#include "SomeUDPApp.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "AddressResolver.h"



Define_Module(SomeUDPApp);

int SomeUDPApp::counter;

void SomeUDPApp::initialize(int stage)
{
    // because of AddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != INITSTAGE_APPLICATION_LAYER)
        return;

    counter = 0;
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);

    localPort = par("localPort");
    destPort = par("destPort");
    msgLength = par("messageLength");

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
        destAddresses.push_back(AddressResolver().resolve(token));

    if (destAddresses.empty())
        return;

    bindToPort(localPort);

    cMessage *timer = new cMessage("sendTimer");
    scheduleAt((double)par("sendInterval"), timer);
}

Address SomeUDPApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    return destAddresses[k];
}

void SomeUDPApp::sendPacket()
{
    char msgName[32];
    sprintf(msgName,"SomeUDPAppData-%d", counter++);

    cMessage *payload = new cMessage(msgName);
    payload->setLength(msgLength);

    Address destAddr = chooseDestAddr();
    sendToUDP(payload, localPort, destAddr, destPort);

    numSent++;
}

void SomeUDPApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // send, then reschedule next sending
        sendPacket();
        scheduleAt(simTime()+(double)par("sendInterval"), msg);
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


void SomeUDPApp::processPacket(cMessage *msg)
{
    EV << "Received packet: ";
    printPacket(msg);
    delete msg;

    numReceived++;
}

