//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/common/INETDefs.h"

#include "SomeUDPApp.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "AddressResolver.h"



Define_Module(SomeUDPApp);

int SomeUDPApp::counter;

void SomeUDPApp::initialize(int stage)
{
    // because of AddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage == INITSTAGE_APPLICATION_LAYER) {
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
        scheduleAfter((double)par("sendInterval"), timer);
    }
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
        scheduleAfter((double)par("sendInterval"), msg);
    }
    else
    {
        // process incoming packet
        processPacket(msg);
    }
}

void SomeUDPApp::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    displayString().setTagArg("t",0,buf);
}

void SomeUDPApp::processPacket(cMessage *msg)
{
    EV << "Received packet: ";
    printPacket(msg);
    delete msg;

    numReceived++;
}

