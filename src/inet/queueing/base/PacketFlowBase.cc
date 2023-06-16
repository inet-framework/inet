//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketFlowBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void PacketFlowBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        producer.reference(inputGate, false);
        consumer.reference(outputGate, false);
        provider.reference(inputGate, false);
        collector.reference(outputGate, false);
        collection.reference(inputGate, false);
        WATCH(inProgressStreamId);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketFlowBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketFlowBase::checkPacketStreaming(Packet *packet)
{
    if (inProgressStreamId != -1 && (packet == nullptr || packet->getTreeId() != inProgressStreamId))
        throw cRuntimeError("Another packet streaming operation is already in progress");
}

void PacketFlowBase::startPacketStreaming(Packet *packet)
{
    inProgressStreamId = packet->getTreeId();
}

void PacketFlowBase::endPacketStreaming(Packet *packet)
{
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
}

bool PacketFlowBase::canPushSomePacket(const cGate *gate) const
{
    return consumer == nullptr || consumer.canPushSomePacket();
}

bool PacketFlowBase::canPushPacket(Packet *packet, const cGate *gate) const
{
    return consumer == nullptr || consumer.canPushPacket(packet);
}

void PacketFlowBase::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    checkPacketStreaming(nullptr);
    emit(packetPushedInSignal, packet);
    processPacket(packet);
    handlePacketProcessed(packet);
    emit(packetPushedOutSignal, packet);
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void PacketFlowBase::pushPacketStart(Packet *packet, const cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    EV_INFO << "Starting packet streaming" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    checkPacketStreaming(packet);
    emit(packetPushedInSignal, packet);
    startPacketStreaming(packet);
    processPacket(packet);
    pushOrSendPacketStart(packet, outputGate, consumer, datarate, packet->getTransmissionId());
    updateDisplayString();
}

void PacketFlowBase::pushPacketEnd(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacketEnd");
    EV_INFO << "Ending packet streaming" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    processPacket(packet);
    emit(packetPushedOutSignal, packet);
    endPacketStreaming(packet);
    pushOrSendPacketEnd(packet, outputGate, consumer, packet->getTransmissionId());
    updateDisplayString();
}

void PacketFlowBase::pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    EV_INFO << "Progressing packet streaming" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    bool isPacketEnd = packet->getDataLength() == position + extraProcessableLength;
    processPacket(packet);
    if (isPacketEnd) {
        emit(packetPushedOutSignal, packet);
        endPacketStreaming(packet);
        pushOrSendPacketEnd(packet, outputGate, consumer, packet->getTransmissionId());
    }
    else
        pushOrSendPacketProgress(packet, outputGate, consumer, datarate, position, extraProcessableLength, packet->getTransmissionId());
    updateDisplayString();
}

void PacketFlowBase::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer.handleCanPushPacketChanged();
}

void PacketFlowBase::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    endPacketStreaming(packet);
    if (producer != nullptr)
        producer.handlePushPacketProcessed(packet, successful);
}

bool PacketFlowBase::canPullSomePacket(const cGate *gate) const
{
    return provider != nullptr && provider.canPullSomePacket();
}

Packet *PacketFlowBase::canPullPacket(const cGate *gate) const
{
    return provider != nullptr ? provider.canPullPacket() : nullptr;
}

Packet *PacketFlowBase::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    checkPacketStreaming(nullptr);
    auto packet = provider.pullPacket();
    take(packet);
    emit(packetPulledInSignal, packet);
    processPacket(packet);
    handlePacketProcessed(packet);
    emit(packetPulledOutSignal, packet);
    if (collector != nullptr)
        animatePullPacket(packet, outputGate, collector.getReferencedGate());
    updateDisplayString();
    return packet;
}

Packet *PacketFlowBase::pullPacketStart(const cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    checkPacketStreaming(nullptr);
    auto packet = provider.pullPacketStart(datarate);
    take(packet);
    emit(packetPulledInSignal, packet);
    inProgressStreamId = packet->getTreeId();
    processPacket(packet);
    emit(packetPulledOutSignal, packet);
    if (collector != nullptr)
        animatePullPacketStart(packet, outputGate, collector.getReferencedGate(), datarate, packet->getTransmissionId());
    updateDisplayString();
    return packet;
}

Packet *PacketFlowBase::pullPacketEnd(const cGate *gate)
{
    Enter_Method("pullPacketEnd");
    auto packet = provider.pullPacketEnd();
    take(packet);
    checkPacketStreaming(packet);
    emit(packetPulledInSignal, packet);
    processPacket(packet);
    inProgressStreamId = packet->getTreeId();
    emit(packetPulledOutSignal, packet);
    endPacketStreaming(packet);
    if (collector != nullptr)
        animatePullPacketEnd(packet, outputGate, collector.getReferencedGate(), packet->getTransmissionId());
    updateDisplayString();
    return packet;
}

Packet *PacketFlowBase::pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    auto packet = provider.pullPacketProgress(datarate, position, extraProcessableLength);
    take(packet);
    checkPacketStreaming(packet);
    inProgressStreamId = packet->getTreeId();
    bool isPacketEnd = packet->getDataLength() == position + extraProcessableLength;
    processPacket(packet);
    if (isPacketEnd) {
        emit(packetPulledOutSignal, packet);
        endPacketStreaming(packet);
    }
    if (collector != nullptr)
        animatePullPacketProgress(packet, outputGate, collector.getReferencedGate(), datarate, position, extraProcessableLength, packet->getTransmissionId());
    updateDisplayString();
    return packet;
}

void PacketFlowBase::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr)
        collector.handleCanPullPacketChanged();
}

void PacketFlowBase::handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    endPacketStreaming(packet);
    if (collector != nullptr)
        collector.handlePullPacketProcessed(packet, successful);
}

} // namespace queueing
} // namespace inet

