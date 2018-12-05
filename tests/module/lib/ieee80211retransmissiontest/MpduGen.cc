//
// Copyright (C) 2015 OpenSim Ltd.
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
// Author: Benjamin Seregi
//

#include "MpduGen.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

Define_Module(MpduGen);

void MpduGen::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        destPort = par("destPort");
        selfMsg = new cMessage("Self msg");
    }
}

void MpduGen::socketDataArrived(UdpSocket *socket, Packet *pk)
{
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
}

void MpduGen::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report\n";
    delete indication;
}

void MpduGen::socketClosed(UdpSocket *socket_)
{
    ASSERT(&socket == socket_);
    EV_WARN << "Ignoring UDP socket closed report\n";
    if (operationalState == STOPPING_OPERATION)
        if (!socket.isOpen())
            finishActiveOperation();
}

void MpduGen::sendPackets()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    const char *destAddrStr = par("destAddress");
    const char *packets = par("packets");
    L3Address destAddr = L3AddressResolver().resolve(destAddrStr);
    int len = strlen(packets);
    const char *packetName = par("packetName");
    for (int i = 0; i < len; i++) {
        std::ostringstream str;
        str << packetName << "-" << i;
        Packet *packet = new Packet(str.str().c_str());
        auto payload = makeShared<ByteCountChunk>();
        if (packets[i] == 'L') {
            payload->setLength(B(par("longPacketSize")));
        }
        else if (packets[i] == 'S') {
            payload->setLength(B(par("shortPacketSize")));
        }
        else
            throw cRuntimeError("Unknown packet type = %c", packets[i]);
        packet->insertAtBack(payload);
        emit(packetSentSignal, packet);
        socket.sendTo(packet, destAddr, destPort);
        numSent++;
    }
    socket.close();
}

void MpduGen::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        sendPackets();
    }
    else
        socket.processMessage(msg);

    if (hasGUI()) {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void MpduGen::handleStartOperation(LifecycleOperation *operation)
{
    scheduleAt(simTime() + par("startTime"), selfMsg);
}

void MpduGen::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void MpduGen::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.destroy();
}

}   // namespace inet

