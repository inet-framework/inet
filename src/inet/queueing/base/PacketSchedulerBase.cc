//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketSchedulerBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        for (int i = 0; i < gateSize("in"); i++) {
            auto inputGate = gate("in", i);
            inputGates.push_back(inputGate);
            auto provider = findConnectedModule<IPassivePacketSource>(inputGate);
            providers.push_back(provider);
            auto producer = findConnectedModule<IActivePacketSource>(inputGate);
            producers.push_back(producer);
        }
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (int i = 0; i < (int)inputGates.size(); i++)
            checkPacketOperationSupport(inputGates[i]);
        checkPacketOperationSupport(outputGate);
    }
}

int PacketSchedulerBase::callSchedulePacket() const
{
    // KLUDGE:
    int index = const_cast<PacketSchedulerBase *>(this)->schedulePacket();
    if (index < 0 || static_cast<unsigned int>(index) >= inputGates.size())
        throw cRuntimeError("Scheduled packet from invalid input gate: %d", index);
    return index;
}

void PacketSchedulerBase::checkPacketStreaming(Packet *packet)
{
    if (inProgressStreamId != -1 && (packet == nullptr || packet->getTreeId() != inProgressStreamId))
        throw cRuntimeError("Another packet streaming operation is already in progress");
}

void PacketSchedulerBase::startPacketStreaming()
{
    inProgressGateIndex = callSchedulePacket();
}

void PacketSchedulerBase::endPacketStreaming(Packet *packet)
{
    emit(packetPulledSignal, packet);
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
    inProgressGateIndex = -1;
}

bool PacketSchedulerBase::canPushSomePacket(cGate *gate) const
{
    int index = callSchedulePacket();
    return index == gate->getIndex();
}

bool PacketSchedulerBase::canPushPacket(Packet *packet, cGate *gate) const
{
    return canPushSomePacket(gate);
}

void PacketSchedulerBase::pushPacket(Packet *packet, cGate *gate)
{
    int index = callSchedulePacket();
    if (index != gate->getIndex())
        throw cRuntimeError("Scheduled packet from wrong input gate");
    consumer->pushPacket(packet, outputGate->getPathEndGate());
}

void PacketSchedulerBase::handleCanPushPacket(cGate *gate)
{
    int index = callSchedulePacket();
    producers[index]->handleCanPushPacket(inputGates[index]->getPathStartGate());
}

bool PacketSchedulerBase::canPullSomePacket(cGate *gate) const
{
    for (int i = 0; i < (int)inputGates.size(); i++) {
        auto inputProvider = providers[i];
        if (inputProvider->canPullSomePacket(inputGates[i]->getPathStartGate()))
            return true;
    }
    return false;
}

Packet *PacketSchedulerBase::canPullPacket(cGate *gate) const
{
    for (int i = 0; i < (int)inputGates.size(); i++) {
        auto inputProvider = providers[i];
        auto packet = inputProvider->canPullPacket(inputGates[i]->getPathStartGate());
        if (packet != nullptr)
            return packet;
    }
    return nullptr;
}

Packet *PacketSchedulerBase::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    checkPacketStreaming(nullptr);
    int index = callSchedulePacket();
    auto packet = providers[index]->pullPacket(inputGates[index]->getPathStartGate());
    take(packet);
    EV_INFO << "Scheduling packet " << packet->getName() << ".\n";
    handlePacketProcessed(packet);
    emit(packetPulledSignal, packet);
    animateSendPacket(packet, outputGate);
    updateDisplayString();
    return packet;
}

Packet *PacketSchedulerBase::pullPacketStart(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    checkPacketStreaming(nullptr);
    startPacketStreaming();
    auto packet = providers[inProgressGateIndex]->pullPacketStart(inputGates[inProgressGateIndex]->getPathStartGate(), datarate);
    take(packet);
    inProgressStreamId = packet->getTreeId();
    animateSendPacketStart(packet, outputGate, datarate);
    updateDisplayString();
    return packet;
}

Packet *PacketSchedulerBase::pullPacketEnd(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketEnd");
    if (!isStreamingPacket())
        startPacketStreaming();
    auto packet = providers[inProgressGateIndex]->pullPacketEnd(inputGates[inProgressGateIndex]->getPathStartGate(), datarate);
    take(packet);
    checkPacketStreaming(packet);
    inProgressStreamId = packet->getTreeId();
    endPacketStreaming(packet);
    animateSendPacketEnd(packet, outputGate, datarate);
    updateDisplayString();
    return packet;
}

Packet *PacketSchedulerBase::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    if (!isStreamingPacket())
        startPacketStreaming();
    auto packet = providers[inProgressGateIndex]->pullPacketProgress(inputGates[inProgressGateIndex]->getPathStartGate(), datarate, position, extraProcessableLength);
    take(packet);
    checkPacketStreaming(packet);
    inProgressStreamId = packet->getTreeId();
    if (packet->getTotalLength() == position + extraProcessableLength)
        endPacketStreaming(packet);
    animateSendPacketProgress(packet, outputGate, datarate, position, extraProcessableLength);
    updateDisplayString();
    return packet;
}

b PacketSchedulerBase::getPullPacketProcessedLength(Packet *packet, cGate *gate)
{
    checkPacketStreaming(packet);
    auto inputGate = inputGates[inProgressGateIndex];
    auto provider = providers[inProgressGateIndex];
    return provider->getPullPacketProcessedLength(packet, inputGate->getPathStartGate());
}

void PacketSchedulerBase::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

void PacketSchedulerBase::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    collector->handlePullPacketProcessed(packet, outputGate->getPathStartGate(), successful);
    inProgressStreamId = -1;
    inProgressGateIndex = -1;
}

} // namespace queueing
} // namespace inet

