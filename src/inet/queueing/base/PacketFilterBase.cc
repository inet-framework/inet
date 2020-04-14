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
#include "inet/common/Simsignals.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {
namespace queueing {

void PacketFilterBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
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
    emit(packetPushedSignal, packet);
    handlePacketProcessed(packet);
    inProgressStreamId = -1;
}

bool PacketFilterBase::canPushSomePacket(cGate *gate) const
{
    return consumer->canPushSomePacket(outputGate->getPathEndGate());
}

bool PacketFilterBase::canPushPacket(Packet *packet, cGate *gate) const
{
    // TODO: KLUDGE:
    return consumer->canPushPacket(packet, outputGate->getPathEndGate()) || !const_cast<PacketFilterBase *>(this)->matchesPacket(packet);
}

void PacketFilterBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    checkPacketStreaming(nullptr);
    take(packet);
    emit(packetPushedSignal, packet);
    if (matchesPacket(packet)) {
        EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else {
        EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
        dropPacket(packet);
    }
    handlePacketProcessed(packet);
    updateDisplayString();
}

void PacketFilterBase::pushPacketStart(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    checkPacketStreaming(packet);
    startPacketStreaming(packet);
    if (matchesPacket(packet)) {
        EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
        pushOrSendPacketStart(packet, outputGate, consumer);
    }
    else {
        EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
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
        EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
        pushOrSendPacketEnd(packet, outputGate, consumer);
        endPacketStreaming(packet);
    }
    else {
        EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
        endPacketStreaming(packet);
        dropPacket(packet);
    }
    pushOrSendPacketEnd(packet, outputGate->getPathEndGate(), consumer);
    updateDisplayString();
}

void PacketFilterBase::pushPacketProgress(Packet *packet, cGate *gate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    if (!isStreamingPacket())
        startPacketStreaming(packet);
    else
        checkPacketStreaming(packet);
    if (matchesPacket(packet)) {
        EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
        if (packet->getTotalLength() == position + extraProcessableLength)
            endPacketStreaming(packet);
        pushOrSendPacketProgress(packet, outputGate, consumer, position, extraProcessableLength);
    }
    else {
        EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
        endPacketStreaming(packet);
        dropPacket(packet);
    }
    updateDisplayString();
}

b PacketFilterBase::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    checkPacketStreaming(packet);
    return consumer->getPushPacketProcessedLength(packet, outputGate->getPathEndGate());
}

void PacketFilterBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void PacketFilterBase::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

bool PacketFilterBase::canPullSomePacket(cGate *gate) const
{
    // TODO: KLUDGE:
    auto nonConstThisPtr = const_cast<PacketFilterBase *>(this);
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->canPullPacket(providerGate);
        if (packet == nullptr)
            return false;
        else if (nonConstThisPtr->matchesPacket(packet))
            return true;
        else {
            packet = provider->pullPacket(providerGate);
            const_cast<PacketFilterBase *>(this)->take(packet);
            EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
            nonConstThisPtr->dropPacket(packet);
            nonConstThisPtr->handlePacketProcessed(packet);
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
        handlePacketProcessed(packet);
        if (matchesPacket(packet)) {
            EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
            animateSend(packet, outputGate);
            updateDisplayString();
            emit(packetPulledSignal, packet);
            return packet;
        }
        else {
            EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
            dropPacket(packet);
        }
    }
}

Packet *PacketFilterBase::pullPacketStart(cGate *gate)
{
    Enter_Method("pullPacketStart");
    throw cRuntimeError("Invalid operation");
}

Packet *PacketFilterBase::pullPacketEnd(cGate *gate)
{
    Enter_Method("pullPacketEnd");
    throw cRuntimeError("Invalid operation");
}

Packet *PacketFilterBase::pullPacketProgress(cGate *gate, b position, b extraProcessableLength)
{
    Enter_Method("pullPacketProgress");
    throw cRuntimeError("Invalid operation");
}

b PacketFilterBase::getPullPacketProcessedLength(Packet *packet, cGate *gate)
{
    checkPacketStreaming(packet);
    return provider->getPullPacketProcessedLength(packet, inputGate->getPathStartGate());
}

void PacketFilterBase::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, outputGate->getPathEndGate(), successful);
}

void PacketFilterBase::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

void PacketFilterBase::dropPacket(Packet *packet)
{
    dropPacket(packet, OTHER_PACKET_DROP);
}

void PacketFilterBase::dropPacket(Packet *packet, PacketDropReason reason, int limit)
{
    PacketProcessorBase::dropPacket(packet, reason, limit);
    numDroppedPackets++;
    droppedTotalLength += packet->getTotalLength();
}

const char *PacketFilterBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'd':
            result = std::to_string(numDroppedPackets);
            break;
        case 'k':
            result = droppedTotalLength.str();
            break;
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

