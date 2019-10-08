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
#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketFilterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        auto inputConnectedModule = findConnectedModule(inputGate);
        auto outputConnectedModule = findConnectedModule(outputGate);
        producer = dynamic_cast<IActivePacketSource *>(inputConnectedModule);
        collector = dynamic_cast<IActivePacketSink *>(outputConnectedModule);
        provider = dynamic_cast<IPassivePacketSource *>(inputConnectedModule);
        consumer = dynamic_cast<IPassivePacketSink *>(outputConnectedModule);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (consumer != nullptr) {
            checkPushPacketSupport(inputGate);
            checkPushPacketSupport(outputGate);
        }
        if (provider != nullptr) {
            checkPopPacketSupport(inputGate);
            checkPopPacketSupport(outputGate);
        }
    }
}

void PacketFilterBase::pushPacket(Packet *packet, cGate *gate)
{
    if (matchesPacket(packet)) {
        EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else {
        EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
        dropPacket(packet, OTHER_PACKET_DROP);
    }
}

bool PacketFilterBase::canPopSomePacket(cGate *gate)
{
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->canPopPacket(providerGate);
        if (packet == nullptr)
            return false;
        else if (matchesPacket(packet))
            return true;
        else {
            packet = provider->popPacket(providerGate);
            EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
            dropPacket(packet, OTHER_PACKET_DROP);
        }
    }
}

Packet *PacketFilterBase::popPacket(cGate *gate)
{
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->popPacket(providerGate);
        if (matchesPacket(packet)) {
            EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
            animateSend(packet, outputGate);
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
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate);
}

void PacketFilterBase::handleCanPopPacket(cGate *gate)
{
    if (collector != nullptr)
        collector->handleCanPopPacket(outputGate);
}

} // namespace queueing
} // namespace inet

