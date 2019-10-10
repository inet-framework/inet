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
#include "inet/queueing/base/PacketMarkerBase.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketMarkerBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
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

void PacketMarkerBase::pushPacket(Packet *packet, cGate *gate)
{
    EV_INFO << "Marking packet " << packet->getName() << "." << endl;
    markPacket(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    updateDisplayString();
}

bool PacketMarkerBase::canPopSomePacket(cGate *gate)
{
    return provider->canPopPacket(inputGate->getPathStartGate());
}

Packet *PacketMarkerBase::popPacket(cGate *gate)
{
    auto packet = provider->popPacket(inputGate->getPathStartGate());
    EV_INFO << "Marking packet " << packet->getName() << "." << endl;
    markPacket(packet);
    numProcessedPackets++;
    updateDisplayString();
    animateSend(packet, outputGate);
    return packet;
}

void PacketMarkerBase::handleCanPushPacket(cGate *gate)
{
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate);
}

void PacketMarkerBase::handleCanPopPacket(cGate *gate)
{
    if (collector != nullptr)
        collector->handleCanPopPacket(outputGate);
}

} // namespace queueing
} // namespace inet

