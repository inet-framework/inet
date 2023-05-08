//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketSchedulerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketSchedulerBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        reverseOrder = par("reverseOrder");
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
        for (auto& inputGate : inputGates)
            checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketSchedulerBase::mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f)
{
    if (gate == outputGate) {
        for (auto inputGate : inputGates)
            f(inputGate);
    }
    else if (std::find(inputGates.begin(), inputGates.end(), gate) != inputGates.end())
        f(outputGate);
    else
        throw cRuntimeError("Unknown gate");
}

int PacketSchedulerBase::callSchedulePacket() const
{
    // KLUDGE const_cast
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

void PacketSchedulerBase::handleCanPushPacketChanged(cGate *gate)
{
    int index = schedulePacket();
    if (index != -1) {
        auto producer = producers[index];
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGates[index]->getPathStartGate());
    }
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
    EV_INFO << "Scheduling packet" << EV_FIELD(packet) << EV_ENDL;
    handlePacketProcessed(packet);
    emit(packetPulledSignal, packet);
    animatePullPacket(packet, outputGate);
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
    animatePullPacketStart(packet, outputGate, datarate, packet->getTransmissionId());
    updateDisplayString();
    return packet;
}

Packet *PacketSchedulerBase::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    if (!isStreamingPacket())
        startPacketStreaming();
    auto packet = providers[inProgressGateIndex]->pullPacketEnd(inputGates[inProgressGateIndex]->getPathStartGate());
    take(packet);
    checkPacketStreaming(packet);
    inProgressStreamId = packet->getTreeId();
    endPacketStreaming(packet);
    animatePullPacketEnd(packet, outputGate, packet->getTransmissionId());
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
    animatePullPacketProgress(packet, outputGate, datarate, position, extraProcessableLength, packet->getTransmissionId());
    updateDisplayString();
    return packet;
}

void PacketSchedulerBase::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr && (!isStreamingPacket() || callSchedulePacket() != inProgressGateIndex))
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
}

void PacketSchedulerBase::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    collector->handlePullPacketProcessed(packet, outputGate->getPathEndGate(), successful);
    inProgressStreamId = -1;
    inProgressGateIndex = -1;
}

} // namespace queueing
} // namespace inet

