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
        checkPushOrPopPacketSupport(inputGate);
        checkPushOrPopPacketSupport(outputGate);
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
        dropPacket(packet, OTHER_PACKET_DROP);
    }
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
}

bool PacketFilterBase::canPopSomePacket(cGate *gate) const
{
    // TODO: KLUDGE:
    auto nonConstThisPtr = const_cast<PacketFilterBase *>(this);
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->canPopPacket(providerGate);
        if (packet == nullptr)
            return false;
        else if (nonConstThisPtr->matchesPacket(packet))
            return true;
        else {
            Enter_Method("canPopSomePacket");
            packet = provider->popPacket(providerGate);
            nonConstThisPtr->take(packet);
            EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
            nonConstThisPtr->dropPacket(packet, OTHER_PACKET_DROP);
            nonConstThisPtr->numProcessedPackets++;
            nonConstThisPtr->processedTotalLength += packet->getTotalLength();
            updateDisplayString();
        }
    }
}

Packet *PacketFilterBase::popPacket(cGate *gate)
{
    Enter_Method("popPacket");
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->popPacket(providerGate);
        take(packet);
        numProcessedPackets++;
        processedTotalLength += packet->getTotalLength();
        if (matchesPacket(packet)) {
            EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
            animateSend(packet, outputGate);
            updateDisplayString();
            emit(packetPoppedSignal, packet);
            return packet;
        }
        else {
            EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
            dropPacket(packet, OTHER_PACKET_DROP);
        }
    }
}

void PacketFilterBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate);
}

void PacketFilterBase::handleCanPopPacket(cGate *gate)
{
    Enter_Method("handleCanPopPacket");
    if (collector != nullptr)
        collector->handleCanPopPacket(outputGate);
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

