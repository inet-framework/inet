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


#include "UDPEchoApp.h"

#include "UDPEchoAppMsg_m.h"
#include "UDPControlInfo_m.h"


Define_Module(UDPEchoApp);

simsignal_t UDPEchoApp::roundTripTimeSignal = SIMSIGNAL_NULL;

void UDPEchoApp::initialize(int stage)
{
    UDPBasicApp::initialize(stage);

    if (stage == 0)
    {
        roundTripTimeSignal = registerSignal("roundTripTime");
    }
}

void UDPEchoApp::finish()
{
}

cPacket *UDPEchoApp::createPacket()
{
    char msgName[32];
    sprintf(msgName, "UDPEcho-%d", counter++);

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
    emit(rcvdPkSignal, packet);

    if (packet->getIsRequest())
    {
        // send back as response
        packet->setIsRequest(false);
        UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(packet->removeControlInfo());
        IPvXAddress srcAddr = ctrl->getSrcAddr();
        int srcPort = ctrl->getSrcPort();
        delete ctrl;

        emit(sentPkSignal, packet);
        socket.sendTo(packet, srcAddr, srcPort);
    }
    else
    {
        // response packet: compute round-trip time
        simtime_t rtt = simTime() - packet->getCreationTime();
        EV << "RTT: " << rtt << "\n";
        emit(roundTripTimeSignal, rtt);
        delete msg;
    }
    numReceived++;
}

