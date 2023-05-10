//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketClassifierBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void PacketClassifierBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        reverseOrder = par("reverseOrder");
        inputGate = gate("in");
        producer.reference(inputGate, false);
        provider.reference(inputGate, false);
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            outputGates.push_back(outputGate);
            PassivePacketSinkRef consumer;
            consumer.reference(outputGate, false);
            consumers.push_back(consumer);
            ActivePacketSinkRef collector;
            collector.reference(outputGate, false);
            collectors.push_back(collector);
        }
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (int i = 0; i < gateSize("out"); i++)
            checkPacketOperationSupport(outputGates[i]);
        checkPacketOperationSupport(inputGate);
    }
}

void PacketClassifierBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketClassifierBase::mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f)
{
    if (gate == inputGate) {
        for (auto outputGate : outputGates)
            f(outputGate);
    }
    else if (std::find(outputGates.begin(), outputGates.end(), gate) != outputGates.end())
        f(inputGate);
    else
        throw cRuntimeError("Unknown gate");
}

int PacketClassifierBase::callClassifyPacket(Packet *packet) const
{
    // KLUDGE
    int index = const_cast<PacketClassifierBase *>(this)->classifyPacket(packet);
    if (index < 0 || static_cast<unsigned int>(index) >= outputGates.size())
        throw cRuntimeError("Packet is classified to invalid output gate: %d", index);
    return index;
}

void PacketClassifierBase::checkPacketStreaming(Packet *packet)
{
    if (inProgressStreamId != -1 && (packet == nullptr || packet->getTreeId() != inProgressStreamId))
        throw cRuntimeError("Another packet streaming operation is already in progress");
}

void PacketClassifierBase::startPacketStreaming(Packet *packet)
{
    EV_INFO << "Classifying packet" << EV_FIELD(packet) << EV_ENDL;
    inProgressStreamId = packet->getTreeId();
    inProgressGateIndex = callClassifyPacket(packet);
}

void PacketClassifierBase::endPacketStreaming(Packet *packet)
{
    emit(packetPushedSignal, packet);
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
    inProgressGateIndex = -1;
}

bool PacketClassifierBase::canPushSomePacket(const cGate *gate) const
{
    for (int i = 0; i < (int)outputGates.size(); i++)
        if (consumers[i]->canPushSomePacket(outputGates[i]->getPathEndGate()))
            return true;
    return false;
}

bool PacketClassifierBase::canPushPacket(Packet *packet, const cGate *gate) const
{
    int index = callClassifyPacket(packet);
    return consumers[index]->canPushPacket(packet, outputGates[index]->getPathEndGate());
}

void PacketClassifierBase::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    checkPacketStreaming(nullptr);
    EV_INFO << "Classifying packet" << EV_FIELD(packet) << EV_ENDL;
    int index = callClassifyPacket(packet);
    handlePacketProcessed(packet);
    emit(packetPushedSignal, packet);
    pushOrSendPacket(packet, outputGates[index], consumers[index].getReferencedGate(), consumers[index]);
    updateDisplayString();
}

void PacketClassifierBase::pushPacketStart(Packet *packet, const cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    pushOrSendPacketStart(packet, outputGates[inProgressGateIndex], consumers[inProgressGateIndex].getReferencedGate(), consumers[inProgressGateIndex], datarate, packet->getTransmissionId());
    updateDisplayString();
}

void PacketClassifierBase::pushPacketEnd(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    auto outputGate = outputGates[inProgressGateIndex];
    auto consumer = consumers[inProgressGateIndex];
    auto consumerGate = consumers[inProgressGateIndex].getReferencedGate();
    endPacketStreaming(packet);
    pushOrSendPacketEnd(packet, outputGate, consumerGate, consumer, packet->getTransmissionId());
    updateDisplayString();
}

void PacketClassifierBase::pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    auto outputGate = outputGates[inProgressGateIndex];
    auto consumer = consumers[inProgressGateIndex];
    if (packet->getTotalLength() == position + extraProcessableLength)
        endPacketStreaming(packet);
    pushOrSendPacketProgress(packet, outputGate, consumers[inProgressGateIndex].getReferencedGate(), consumer, datarate, position, extraProcessableLength, packet->getTransmissionId());
    updateDisplayString();
}

void PacketClassifierBase::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(producer.getReferencedGate());
}

void PacketClassifierBase::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    producer->handlePushPacketProcessed(packet, producer.getReferencedGate(), successful);
}

bool PacketClassifierBase::canPullSomePacket(const cGate *gate) const
{
    return canPullPacket(gate) != nullptr;
}

Packet *PacketClassifierBase::canPullPacket(const cGate *gate) const
{
    auto packet = provider->canPullPacket(provider.getReferencedGate());
    if (packet == nullptr)
        return nullptr;
    else {
        int index = callClassifyPacket(packet);
        return index == gate->getIndex() ? packet : nullptr;
    }
}

Packet *PacketClassifierBase::pullPacket(const cGate *gate)
{
    auto packet = provider->pullPacket(provider.getReferencedGate());
    int index = callClassifyPacket(packet);
    if (index != gate->getIndex())
        throw cRuntimeError("Packet is classified to the wrong output gate (%d) when pulled from gate (%d)", index, gate->getIndex());
    return packet;
}

void PacketClassifierBase::handleCanPullPacketChanged(const cGate *gate)
{
    auto packet = provider->canPullPacket(provider.getReferencedGate());
    if (packet != nullptr) {
        int index = callClassifyPacket(packet);
        auto collector = collectors[index];
        if (collector != nullptr)
            collector->handleCanPullPacketChanged(outputGates[index]->getPathEndGate());
    }
}

} // namespace queueing
} // namespace inet

