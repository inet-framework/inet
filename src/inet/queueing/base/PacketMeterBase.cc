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
#include "inet/queueing/base/PacketMeterBase.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketMeterBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPushOrPopPacketSupport(inputGate);
        checkPushOrPopPacketSupport(outputGate);
    }
}

void PacketMeterBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    EV_INFO << "Metering packet " << packet->getName() << "." << endl;
    meterPacket(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
}

bool PacketMeterBase::canPopSomePacket(cGate *gate) const
{
    return provider->canPopPacket(inputGate->getPathStartGate());
}

Packet *PacketMeterBase::popPacket(cGate *gate)
{
    Enter_Method("popPacket");
    auto packet = provider->popPacket(inputGate->getPathStartGate());
    take(packet);
    EV_INFO << "Metering packet " << packet->getName() << "." << endl;
    meterPacket(packet);
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
    animateSend(packet, outputGate);
    return packet;
}

void PacketMeterBase::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate);
}

void PacketMeterBase::handleCanPopPacket(cGate *gate)
{
    Enter_Method("handleCanPopPacket");
    if (collector != nullptr)
        collector->handleCanPopPacket(outputGate);
}

} // namespace queueing
} // namespace inet

