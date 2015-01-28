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

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/tunapp/TunBasicApp.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/transportlayer/udp/UDPPacket.h"

#define MSGKIND_START  0
#define MSGKIND_SEND  1

namespace inet {

Define_Module(TunBasicApp);

simsignal_t TunBasicApp::sentPkSignal = registerSignal("sentPk");
simsignal_t TunBasicApp::rcvdPkSignal = registerSignal("rcvdPk");

void TunBasicApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // parameters
        packetsReceivedViaUdp = 0;
        packetsToSend = par("packetsToSend");
        packetsReceivedViaTun = 0;
        packetsSentOverUdp = 0;
        localPort = par("localPort");
        remotePort = par("remotePort");
        localAddress = L3Address(par("localAddress"));
        remoteAddress = L3Address(par("remoteAddress"));

        timeMsg = new cMessage("TunAppTimer");
        timeMsg->setKind(MSGKIND_START);
        scheduleAt((simtime_t)par("startTime"), timeMsg);
    }
}


void TunBasicApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    } else {
        if (msg->getArrivalGate()->isName("tunIn")) {
            EV_INFO << "msg " << msg->getName() << " arrived from tun. " << packetsReceivedViaTun + 1 << " packets received so far\n";
            packetsReceivedViaTun++;
            IPv4Datagram *ip = check_and_cast<IPv4Datagram *>(msg);
            if (msg) {
                UDPPacket *udp = check_and_cast<UDPPacket *>(ip->decapsulate());
                udp->setDestinationPort(localPort);
                udp->setSourcePort(remotePort);
                IPv4Datagram *echoMsg = ip->dup();
                echoMsg->setSrcAddress(ip->getDestAddress());
                echoMsg->setDestAddress(ip->getSrcAddress());
                echoMsg->encapsulate(udp);
                send(echoMsg, "tunOut");
            }
        } else if (msg->getArrivalGate()->isName("udpIn")) {
            EV_INFO << "msg "  << msg->getName() << " arrived from UDP. " << packetsReceivedViaUdp + 1 << " packets received so far\n";
            packetsReceivedViaUdp++;
        } else {
            EV_DEBUG << "unknown arrivalGate\n";
        }
        delete msg;
    }
}

void TunBasicApp::sendPacket()
{
    char msgName[32];
    sprintf(msgName, "TunData-%d", ++packetsSentOverUdp);
    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(par("messageLength").longValue());
    emit(sentPkSignal, payload);
    socket.sendTo(payload, remoteAddress, remotePort, nullptr);
}


void TunBasicApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_START: {
            socket.setOutputGate(gate("udpOut"));
            socket.bind(localPort);
            sendPacket();
            if (packetsSentOverUdp < packetsToSend) {
                timeMsg->setKind(MSGKIND_SEND);
                scheduleAt(simulation.getSimTime() + (simtime_t)par("thinkTime"), timeMsg);
            }
            break;
        }
        case MSGKIND_SEND: {
            if (packetsSentOverUdp < packetsToSend) {
                sendPacket();
                timeMsg->setKind(MSGKIND_SEND);
                scheduleAt(simulation.getSimTime() + (simtime_t)par("thinkTime"), timeMsg);
            } else {
                socket.close();
            }
            break;
        }

        default: EV_DEBUG << "Unknown message kind\n";
    }
}

void TunBasicApp::finish()
{
    EV_INFO << "packets sent to UDP: " << packetsSentOverUdp << endl;
    EV_INFO << "packets received via UDP: " << packetsReceivedViaUdp << endl;
    EV_INFO << "packets received via Tun: " << packetsReceivedViaTun << endl;
}

TunBasicApp::~TunBasicApp()
{
    delete timeMsg;
}

} // namespace inet
