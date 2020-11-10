//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/queueing/server/PreemptingServer.h"

namespace inet {
namespace queueing {

Define_Module(PreemptingServer);

void PreemptingServer::initialize(int stage)
{
    PacketServerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        datarate = bps(par("datarate"));
        timer = new cMessage("Timer");
    }
}

void PreemptingServer::handleMessage(cMessage *message)
{
    if (message == timer)
        endStreaming();
    else
        PacketServerBase::handleMessage(message);
}

bool PreemptingServer::canStartStreaming() const
{
    return provider->canPullSomePacket(inputGate->getPathStartGate()) && consumer->canPushSomePacket(outputGate->getPathEndGate());
}

void PreemptingServer::startStreaming()
{
    auto packet = provider->pullPacketStart(inputGate->getPathStartGate(), datarate);
    take(packet);
    EV_INFO << "Starting streaming packet" << EV_FIELD(packet) << EV_ENDL;
    streamedPacket = packet;
    pushOrSendPacketStart(streamedPacket->dup(), outputGate, consumer, datarate, packet->getTransmissionId());
    scheduleAt(simTime() + s(streamedPacket->getTotalLength() / datarate).get(), timer);
    handlePacketProcessed(streamedPacket);
    updateDisplayString();
}

void PreemptingServer::endStreaming()
{
    auto packet = provider->pullPacketEnd(inputGate->getPathStartGate());
    take(packet);
    delete streamedPacket;
    streamedPacket = packet;
    EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
    pushOrSendPacketEnd(streamedPacket, outputGate, consumer, packet->getTransmissionId());
    streamedPacket = nullptr;
    updateDisplayString();
}

void PreemptingServer::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (!isStreaming() && canStartStreaming())
        startStreaming();
}

void PreemptingServer::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (isStreaming()) {
        endStreaming();
        cancelEvent(timer);
    }
    else if (canStartStreaming())
        startStreaming();
}

void PreemptingServer::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (isStreaming()) {
        delete streamedPacket;
        streamedPacket = provider->pullPacketEnd(inputGate->getPathStartGate());
        take(streamedPacket);
        EV_INFO << "Ending streaming packet" << EV_FIELD(packet, *streamedPacket) << EV_ENDL;
        delete streamedPacket;
        streamedPacket = nullptr;
    }
}

} // namespace queueing
} // namespace inet
