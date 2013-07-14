//
// copyright (C) 2012 Kyeong Soo (Joseph) Kim
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
#include "UDPBurstApp.h"
#include "UDPControlInfo_m.h"
#include "IPAddressResolver.h"


Define_Module(UDPBurstApp);

UDPBurstApp::UDPBurstApp()
{
}

UDPBurstApp::~UDPBurstApp()
{
    cancelAndDelete(burstTimer);
    cancelAndDelete(messageTimer);
}

void UDPBurstApp::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage!=3)
        return;

    counter = 0;
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);

    localPort = par("localPort");
    destPort = par("destPort");
    lineRate = par("lineRate").doubleValue();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
        destAddresses.push_back(IPAddressResolver().resolve(token));

    if (destAddresses.empty())
        return;

    bindToPort(localPort);

    burstTimer = new cMessage("burstTimer", 0);
    messageTimer = new cMessage("messageTimer", 1);
    scheduleAt((double)par("startTime"), burstTimer);

    // send a small packet to trigger ARP process
    cPacket *payload = new cPacket("Triggering ARP process");
    payload->setByteLength(0);  // no payload
    IPvXAddress destAddr = chooseDestAddr();
    sendToUDP(payload, localPort, destAddr, destPort);
}

cPacket *UDPBurstApp::createPacket(int payloadLength)
{
    char msgName[32];
    sprintf(msgName,"UDPBurstAppData-%d", counter++);

    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(payloadLength);
    return payload;
}

void UDPBurstApp::sendPacket() {
    int payloadLength = (messageLength > UDP_MAX_PAYLOAD) ? UDP_MAX_PAYLOAD : messageLength;
    messageLength -= payloadLength;
    cPacket *payload = createPacket(payloadLength);
    IPvXAddress destAddr = chooseDestAddr();
    sendToUDP(payload, localPort, destAddr, destPort);
    numSent++;
    if (messageLength > 0) {
        scheduleAt(simTime()+ double(payloadLength*8/lineRate), messageTimer);
    }
}

void UDPBurstApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (msg->getKind() == 0)
        {   // start of a new burst; reset messageLength
            messageLength = par("messageLength").longValue();
            scheduleAt(simTime()+(double)par("messagePeriod"), msg);

            // cancel any scheduled messageTimer due to random values of messageLength and messagePeriod;
            // i.e., there could be any message not sent yet during the previous message period.
            cancelEvent(messageTimer);
        }
        sendPacket();
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

//void UDPBurstApp::sendToUDPDelayed(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort, double delay)
//{
//    // send message to UDP, with the appropriate control info attached after a given delay
//    msg->setKind(UDP_C_DATA);
//
//    UDPControlInfo *ctrl = new UDPControlInfo();
//    ctrl->setSrcPort(srcPort);
//    ctrl->setDestAddr(destAddr);
//    ctrl->setDestPort(destPort);
//    msg->setControlInfo(ctrl);
//
//    EV << "Sending packet: ";
//    printPacket(msg);
//
//    sendDelayed(msg, delay, "udpOut");
//}
