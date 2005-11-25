//
// Copyright (C) 2005 Andras Babos
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
#include "UDPEchoApp.h"
#include "UDPEchoAppMsg_m.h"
#include "UDPControlInfo_m.h"


Define_Module(UDPEchoApp);

void UDPEchoApp::initialize(int stage)
{
    UDPBasicApp::initialize(stage);

    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage!=3)
        return;
}

void UDPEchoApp::finish()
{
}

cMessage *UDPEchoApp::createPacket()
{
    char msgName[32];
    sprintf(msgName,"UDPEcho-%d", counter++);

    cMessage *message = new UDPEchoAppMsg(msgName);
    message->setByteLength(msgByteLength);

    return message;
}

void UDPEchoApp::processPacket(cMessage *msg)
{
    UDPEchoAppMsg *packet = check_and_cast<UDPEchoAppMsg *>(msg);

    if (packet->isRequest())
    {
        UDPControlInfo *controlInfo = check_and_cast<UDPControlInfo *>(packet->controlInfo());

        // swap src and dest
        IPvXAddress srcAddr = controlInfo->srcAddr();
        int srcPort = controlInfo->srcPort();
        controlInfo->setSrcAddr(controlInfo->destAddr());
        controlInfo->setSrcPort(controlInfo->destPort());
        controlInfo->setDestAddr(srcAddr);
        controlInfo->setDestPort(srcPort);

        packet->setIsRequest(false);
        send(packet, "to_udp");
    }
    else
    {
        simtime_t rtt = simTime() - packet->creationTime();
        EV << "RTT: " << rtt << "\n";
        delete msg;
    }
    numReceived++;
}



