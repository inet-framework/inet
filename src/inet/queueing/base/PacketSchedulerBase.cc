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
        collector.reference(outputGate, false);
        consumer.reference(outputGate, false);
        for (int i = 0; i < gateSize("in"); i++) {
            auto inputGate = gate("in", i);
            inputGates.push_back(inputGate);
            PassivePacketSourceRef provider;
            provider.reference(inputGate, false);
            providers.push_back(provider);
            ActivePacketSourceRef producer;
            producer.reference(inputGate, false);
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
    int index = schedulePacket();
    if (index < 0 || static_cast<unsigned int>(index) >= inputGates.size())
        throw cRuntimeError("Scheduled packet from invalid input gate: %d", index);
    return index;
}

int PacketSchedulerBase::schedulePushedPacket(Packet *packet, const cGate *gate)
{
    int index = callSchedulePacket();
    if (index != gate->getIndex())
        throw cRuntimeError("Scheduled packet from wrong input gate");
    handlePacketScheduled(packet, index);
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

bool PacketSchedulerBase::canPushSomePacket(const cGate *gate) const
{
    int index = callSchedulePacket();
    return index == gate->getIndex();
}

bool PacketSchedulerBase::canPushPacket(Packet *packet, const cGate *gate) const
{
    return canPushSomePacket(gate);
}

void PacketSchedulerBase::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    schedulePushedPacket(packet, gate);
    consumer.pushPacket(packet);
}

void PacketSchedulerBase::pushPacketStart(Packet *packet, const cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    ASSERT(!isStreamingPacket());
    schedulePushedPacket(packet, gate);
    consumer.pushPacketStart(packet, datarate);
}

void PacketSchedulerBase::pushPacketEnd(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacketEnd");
    ASSERT(isStreamingPacket());
    int index = callSchedulePacket();
    if (index != gate->getIndex())
        throw cRuntimeError("Scheduled packet from wrong input gate");
    endPacketStreaming(packet);
    consumer.pushPacketEnd(packet);
}

void PacketSchedulerBase::pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    ASSERT(isStreamingPacket());
    int index = callSchedulePacket();
    if (index != gate->getIndex())
        throw cRuntimeError("Scheduled packet from wrong input gate");
    consumer.pushPacketProgress(packet, datarate, position, extraProcessableLength);
}

void PacketSchedulerBase::handleCanPushPacketChanged(const cGate *gate)
{
    int index = schedulePacket();
    if (index != -1) {
        auto producer = producers[index];
        if (producer != nullptr)
            producer.handleCanPushPacketChanged();
    }
}

bool PacketSchedulerBase::canPullSomePacket(const cGate *gate) const
{
    return canPullPacket(gate) != nullptr;
}

Packet *PacketSchedulerBase::canPullPacket(const cGate *gate) const
{
    // the packet of the input that the next pull serves
    int index = schedulePacket();
    if (index < 0 || static_cast<unsigned int>(index) >= inputGates.size())
        return nullptr;
    else
        return providers[index].canPullPacket();
}

Packet *PacketSchedulerBase::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    checkPacketStreaming(nullptr);
    int index = callSchedulePacket();
    auto packet = providers[index].pullPacket();
    take(packet);
    handlePacketScheduled(packet, index);
    EV_INFO << "Scheduling packet" << EV_FIELD(packet) << EV_ENDL;
    handlePacketProcessed(packet);
    emit(packetPulledSignal, packet);
    if (collector != nullptr)
        animatePullPacket(packet, outputGate, collector.getReferencedGate());
    return packet;
}

Packet *PacketSchedulerBase::pullPacketStart(const cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    checkPacketStreaming(nullptr);
    startPacketStreaming();
    auto packet = providers[inProgressGateIndex].pullPacketStart(datarate);
    EV_INFO << "Starting packet streaming" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    handlePacketScheduled(packet, inProgressGateIndex);
    inProgressStreamId = packet->getTreeId();
    if (collector != nullptr)
        animatePullPacketStart(packet, outputGate, collector.getReferencedGate(), datarate, packet->getTransmissionId());
    return packet;
}

Packet *PacketSchedulerBase::pullPacketEnd(const cGate *gate)
{
    Enter_Method("pullPacketEnd");
    bool isStarting = !isStreamingPacket();
    if (isStarting)
        startPacketStreaming();
    auto packet = providers[inProgressGateIndex].pullPacketEnd();
    EV_INFO << "Ending packet streaming" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    checkPacketStreaming(packet);
    if (isStarting)
        handlePacketScheduled(packet, inProgressGateIndex);
    inProgressStreamId = packet->getTreeId();
    endPacketStreaming(packet);
    if (collector != nullptr)
        animatePullPacketEnd(packet, outputGate, collector.getReferencedGate(), packet->getTransmissionId());
    return packet;
}

Packet *PacketSchedulerBase::pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    bool isStarting = !isStreamingPacket();
    if (isStarting)
        startPacketStreaming();
    auto packet = providers[inProgressGateIndex].pullPacketProgress(datarate, position, extraProcessableLength);
    EV_INFO << "Progressing packet streaming" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    checkPacketStreaming(packet);
    if (isStarting)
        handlePacketScheduled(packet, inProgressGateIndex);
    inProgressStreamId = packet->getTreeId();
    if (packet->getDataLength() == position + extraProcessableLength)
        endPacketStreaming(packet);
    if (collector != nullptr)
        animatePullPacketProgress(packet, outputGate, collector.getReferencedGate(), datarate, position, extraProcessableLength, packet->getTransmissionId());
    return packet;
}

void PacketSchedulerBase::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr)
        collector.handleCanPullPacketChanged();
}

void PacketSchedulerBase::handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    collector.handlePullPacketProcessed(packet, successful);
    inProgressStreamId = -1;
    inProgressGateIndex = -1;
}

} // namespace queueing
} // namespace inet

