//
// Copyright (C) 2014 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include <stdlib.h>
#include <stdio.h>

#include "inet/applications/tunapp/TunLoopbackApp.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/transportlayer/udp/UDPPacket.h"

namespace inet {

Define_Module(TunLoopbackApp);

simsignal_t TunLoopbackApp::sentPkSignal = registerSignal("sentPk");
simsignal_t TunLoopbackApp::rcvdPkSignal = registerSignal("rcvdPk");

void TunLoopbackApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // parameters
        packetsReceived = 0;
        packetsSent = 0;
    }
}


void TunLoopbackApp::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGate()->isName("tunIn")) {
        EV_INFO << "msg " << msg->getName() << " arrived from tun. " << packetsReceived + 1 << " packets received so far\n";
        packetsReceived++;
        IPv4Datagram *ip = check_and_cast<IPv4Datagram *>(msg);
        if (msg) {
            UDPPacket *udp = check_and_cast<UDPPacket *>(ip->decapsulate());
            int remotePort = udp->getDestinationPort();
            udp->setDestinationPort(udp->getSourcePort());
            udp->setSourcePort(remotePort);
            IPv4Datagram *echoMsg = ip->dup();
            echoMsg->setSrcAddress(ip->getDestAddress());
            echoMsg->setDestAddress(ip->getSrcAddress());
            echoMsg->encapsulate(udp);
            send(echoMsg, "tunOut");
            packetsSent++;
        }
    } else {
        EV_DEBUG << "unknown arrivalGate\n";
    }
    delete msg;
}


void TunLoopbackApp::finish()
{
    EV_INFO << "packets sent: " << packetsSent << endl;
    EV_INFO << "packets received: " << packetsReceived << endl;
}


TunLoopbackApp::~TunLoopbackApp()
{
}

} // namespace inet
