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
#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketSchedulerBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        for (int i = 0; i < gateSize("in"); i++) {
            auto inputGate = gate("in", i);
            auto provider = findConnectedModule<IPassivePacketSource>(inputGate);
            inputGates.push_back(inputGate);
            providers.push_back(provider);
        }
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (int i = 0; i < gateSize("in"); i++)
            checkPopPacketSupport(inputGates[i]);
        checkPopPacketSupport(outputGate);
    }
}

bool PacketSchedulerBase::canPopSomePacket(cGate *gate) const
{
    for (int i = 0; i < gateSize("in"); i++) {
        auto inputProvider = providers[i];
        if (inputProvider->canPopSomePacket(inputGates[i]->getPathStartGate()))
            return true;
    }
    return false;
}

Packet *PacketSchedulerBase::popPacket(cGate *gate)
{
    Enter_Method("popPacket");
    int index = schedulePacket();
    if (index < 0 || static_cast<unsigned int>(index) >= inputGates.size())
        throw cRuntimeError("Scheduled packet from invalid input gate: %d", index);
    auto packet = providers[index]->popPacket(inputGates[index]->getPathStartGate());
    EV_INFO << "Scheduling packet " << packet->getName() << ".\n";
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    animateSend(packet, outputGate);
    emit(packetPoppedSignal, packet);
    return packet;
}

void PacketSchedulerBase::handleCanPopPacket(cGate *gate)
{
    Enter_Method("handleCanPopPacket");
    if (collector != nullptr)
        collector->handleCanPopPacket(outputGate);
}

} // namespace queueing
} // namespace inet

