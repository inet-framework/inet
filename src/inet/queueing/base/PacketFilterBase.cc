//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketFilterBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketFilterBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        backpressure = par("backpressure");
        inputGate = gate("in");
        outputGate = gate("out");
        producer.reference(inputGate, false);
        collector.reference(outputGate, false);
        provider.reference(inputGate, false);
        consumer.reference(outputGate, false);
        numDroppedPackets = 0;
        droppedTotalLength = b(0);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketFilterBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketFilterBase::checkPacketStreaming(Packet *packet)
{
    if (inProgressStreamId != -1 && (packet == nullptr || packet->getTreeId() != inProgressStreamId))
        throw cRuntimeError("Another packet streaming operation is already in progress");
}

void PacketFilterBase::startPacketStreaming(Packet *packet)
{
    inProgressStreamId = packet->getTreeId();
}

void PacketFilterBase::endPacketStreaming(Packet *packet)
{
    emit(packetPushedInSignal, packet);
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
}

bool PacketFilterBase::canPushSomePacket(cGate *gate) const
{
    return consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate());
}

bool PacketFilterBase::canPushPacket(Packet *packet, cGate *gate) const
{
    if (backpressure)
        return matchesPacket(packet) && consumer != nullptr && consumer->canPushPacket(packet, outputGate->getPathEndGate());
    else
        return !matchesPacket(packet) || consumer == nullptr || consumer->canPushPacket(packet, outputGate->getPathEndGate());
}

void PacketFilterBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    checkPacketStreaming(nullptr);
    emit(packetPushedInSignal, packet);
    if (matchesPacket(packet)) {
        processPacket(packet);
        handlePacketProcessed(packet);
        EV_INFO << "Passing through packet" << EV_FIELD(packet) << EV_ENDL;
        emit(packetPushedOutSignal, packet);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else {
        EV_INFO << "Filtering out packet" << EV_FIELD(packet) << EV_ENDL;
        handlePacketProcessed(packet);
        dropPacket(packet);
    }
    updateDisplayString();
}

void PacketFilterBase::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    if (matchesPacket(packet)) {
        processPacket(packet);
        EV_INFO << "Passing through packet" << EV_FIELD(packet) << EV_ENDL;
        pushOrSendPacketStart(packet, outputGate, consumer, datarate, packet->getTransmissionId());
    }
    else {
        EV_INFO << "Filtering out packet" << EV_FIELD(packet) << EV_ENDL;
        dropPacket(packet);
    }
    updateDisplayString();
}

void PacketFilterBase::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    if (matchesPacket(packet)) {
        processPacket(packet);
        EV_INFO << "Passing through packet" << EV_FIELD(packet) << EV_ENDL;
        endPacketStreaming(packet);
        emit(packetPushedOutSignal, packet);
        pushOrSendPacketEnd(packet, outputGate, consumer, packet->getTransmissionId());
    }
    else {
        EV_INFO << "Filtering out packet" << EV_FIELD(packet) << EV_ENDL;
        endPacketStreaming(packet);
        dropPacket(packet);
    }
    updateDisplayString();
}

void PacketFilterBase::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    if (matchesPacket(packet)) {
        processPacket(packet);
        EV_INFO << "Passing through packet" << EV_FIELD(packet) << EV_ENDL;
        if (packet->getTotalLength() == position + extraProcessableLength)
            endPacketStreaming(packet);
        pushOrSendPacketProgress(packet, outputGate, consumer, datarate, position, extraProcessableLength, packet->getTransmissionId());
    }
    else {
        EV_INFO << "Filtering out packet" << EV_FIELD(packet) << EV_ENDL;
        endPacketStreaming(packet);
        dropPacket(packet);
    }
    updateDisplayString();
}

void PacketFilterBase::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PacketFilterBase::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

bool PacketFilterBase::canPullSomePacket(cGate *gate) const
{
    Enter_Method("canPullSomePacket");
    return canPullPacket(gate) != nullptr;
}

Packet *PacketFilterBase::canPullPacket(cGate *gate) const
{
    Enter_Method("canPullPacket");
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->canPullPacket(providerGate);
        if (packet == nullptr)
            return nullptr;
        else if (matchesPacket(packet))
            return packet;
        else if (backpressure)
            return nullptr;
        else {
            auto nonConstThisPtr = const_cast<PacketFilterBase *>(this);
            packet = provider->pullPacket(providerGate);
            nonConstThisPtr->take(packet);
            nonConstThisPtr->emit(packetPulledInSignal, packet);
            EV_INFO << "Filtering out packet" << EV_FIELD(packet) << EV_ENDL;
            // KLUDGE
            nonConstThisPtr->handlePacketProcessed(packet);
            nonConstThisPtr->dropPacket(packet);
            updateDisplayString();
        }
    }
}

Packet *PacketFilterBase::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->pullPacket(providerGate);
        take(packet);
        emit(packetPulledInSignal, packet);
        if (matchesPacket(packet)) {
            processPacket(packet);
            handlePacketProcessed(packet);
            EV_INFO << "Passing through packet" << EV_FIELD(packet) << EV_ENDL;
            animatePullPacket(packet, outputGate);
            updateDisplayString();
            emit(packetPulledOutSignal, packet);
            return packet;
        }
        else {
            EV_INFO << "Filtering out packet" << EV_FIELD(packet) << EV_ENDL;
            handlePacketProcessed(packet);
            dropPacket(packet);
        }
    }
}

Packet *PacketFilterBase::pullPacketStart(cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    throw cRuntimeError("Invalid operation");
}

Packet *PacketFilterBase::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    throw cRuntimeError("Invalid operation");
}

Packet *PacketFilterBase::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    throw cRuntimeError("Invalid operation");
}

void PacketFilterBase::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, outputGate->getPathEndGate(), successful);
}

void PacketFilterBase::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr)
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
}

void PacketFilterBase::dropPacket(Packet *packet)
{
    dropPacket(packet, OTHER_PACKET_DROP);
}

void PacketFilterBase::dropPacket(Packet *packet, PacketDropReason reason, int limit)
{
    numDroppedPackets++;
    droppedTotalLength += packet->getTotalLength();
    PacketProcessorBase::dropPacket(packet, reason, limit);
}

std::string PacketFilterBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'd':
            return std::to_string(numDroppedPackets);
        case 'k':
            return droppedTotalLength.str();
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
}

} // namespace queueing
} // namespace inet

