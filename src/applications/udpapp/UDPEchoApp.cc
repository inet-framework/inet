//
// Copyright (C) 2005 Andras Babos
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

cPacket *UDPEchoApp::createPacket()
{
    char msgName[32];
    sprintf(msgName,"UDPEcho-%d", counter++);

    UDPEchoAppMsg *message = new UDPEchoAppMsg(msgName);
    message->setByteLength(par("messageLength").longValue());

    return message;
}

void UDPEchoApp::processPacket(cPacket *msg)
{
    if (msg->getKind() == UDP_I_ERROR)
    {
        delete msg;
        return;
    }

    UDPEchoAppMsg *packet = check_and_cast<UDPEchoAppMsg *>(msg);

    if (packet->getIsRequest())
    {
        UDPControlInfo *controlInfo = check_and_cast<UDPControlInfo *>(packet->getControlInfo());

        // swap src and dest
        IPvXAddress srcAddr = controlInfo->getSrcAddr();
        int srcPort = controlInfo->getSrcPort();
        controlInfo->setSrcAddr(controlInfo->getDestAddr());
        controlInfo->setSrcPort(controlInfo->getDestPort());
        controlInfo->setDestAddr(srcAddr);
        controlInfo->setDestPort(srcPort);

        packet->setIsRequest(false);
        send(packet, "udpOut");
    }
    else
    {
        simtime_t rtt = simTime() - packet->getCreationTime();
        EV << "RTT: " << rtt << "\n";
        delete msg;
    }
    numReceived++;
}



