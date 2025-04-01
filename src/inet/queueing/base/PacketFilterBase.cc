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

bool PacketFilterBase::canPushSomePacket(const cGate *gate) const
{
    return consumer == nullptr || consumer.canPushSomePacket();
}

bool PacketFilterBase::canPushPacket(Packet *packet, const cGate *gate) const
{
    if (backpressure)
        return matchesPacket(packet) && consumer != nullptr && consumer.canPushPacket(packet);
    else
        return !matchesPacket(packet) || consumer == nullptr || consumer.canPushPacket(packet);
}

void PacketFilterBase::pushPacket(Packet *packet, const cGate *gate)
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
}

void PacketFilterBase::pushPacketStart(Packet *packet, const cGate *gate, bps datarate)
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
}

void PacketFilterBase::pushPacketEnd(Packet *packet, const cGate *gate)
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
}

void PacketFilterBase::pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength)
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
}

void PacketFilterBase::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer.handleCanPushPacketChanged();
}

void PacketFilterBase::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    producer.handlePushPacketProcessed(packet, successful);
}

bool PacketFilterBase::canPullSomePacket(const cGate *gate) const
{
    Enter_Method("canPullSomePacket");
    return canPullPacket(gate) != nullptr;
}

Packet *PacketFilterBase::canPullPacket(const cGate *gate) const
{
    Enter_Method("canPullPacket");
    while (true) {
        auto packet = provider.canPullPacket();
        if (packet == nullptr)
            return nullptr;
        else if (matchesPacket(packet))
            return packet;
        else if (backpressure)
            return nullptr;
        else {
            auto nonConstThisPtr = const_cast<PacketFilterBase *>(this);
            packet = nonConstThisPtr->provider.pullPacket();
            nonConstThisPtr->take(packet);
            nonConstThisPtr->emit(packetPulledInSignal, packet);
            EV_INFO << "Filtering out packet" << EV_FIELD(packet) << EV_ENDL;
            // KLUDGE
            nonConstThisPtr->handlePacketProcessed(packet);
            nonConstThisPtr->dropPacket(packet);
        }
    }
}

Packet *PacketFilterBase::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    while (true) {
        auto packet = provider.pullPacket();
        take(packet);
        emit(packetPulledInSignal, packet);
        if (matchesPacket(packet)) {
            processPacket(packet);
            handlePacketProcessed(packet);
            EV_INFO << "Passing through packet" << EV_FIELD(packet) << EV_ENDL;
            if (collector != nullptr)
                animatePullPacket(packet, outputGate, collector.getReferencedGate());
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

Packet *PacketFilterBase::pullPacketStart(const cGate *gate, bps datarate)
{
    Enter_Method("pullPacketStart");
    throw cRuntimeError("Invalid operation");
}

Packet *PacketFilterBase::pullPacketEnd(const cGate *gate)
{
    Enter_Method("pullPacketEnd");
    throw cRuntimeError("Invalid operation");
}

Packet *PacketFilterBase::pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    throw cRuntimeError("Invalid operation");
}

void PacketFilterBase::handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    if (collector != nullptr)
        collector.handlePullPacketProcessed(packet, successful);
}

void PacketFilterBase::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr)
        collector.handleCanPullPacketChanged();
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

