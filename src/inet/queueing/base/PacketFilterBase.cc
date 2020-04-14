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

void PacketFilterBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
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
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
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
            nonConstThisPtr->numProcessedPackets++;
            nonConstThisPtr->processedTotalLength += packet->getTotalLength();
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
        numProcessedPackets++;
        processedTotalLength += packet->getTotalLength();
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

void PacketFilterBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
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
    PacketQueueingElementBase::dropPacket(packet, reason, limit);
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

