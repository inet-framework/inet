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

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
        destAddresses.push_back(IPAddressResolver().resolve(token));

    if (destAddresses.empty())
        return;

    bindToPort(localPort);

    cMessage *timer = new cMessage("sendTimer");
    scheduleAt((double)par("startupTime"), timer);
}

cPacket *UDPBurstApp::createPacket(int payloadLength)
{
    char msgName[32];
    sprintf(msgName,"UDPBurstAppData-%d", counter++);

    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(payloadLength);
    return payload;
}

void UDPBurstApp::sendPacket()
{
    int messageLength = par("messageLength").longValue();
    
    do
    {
        int payloadLength = (messageLength > UDP_MAX_PAYLOAD) ? UDP_MAX_PAYLOAD : messageLength;
        messageLength -= payloadLength;
        cPacket *payload = createPacket(payloadLength);
        IPvXAddress destAddr = chooseDestAddr();
        sendToUDP(payload, localPort, destAddr, destPort);

        numSent++;
    } while (messageLength > 0);
}
