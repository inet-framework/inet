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
        for (int i = 0; i < (int)inputGates.size(); i++)
            checkPacketOperationSupport(inputGates[i]);
        checkPacketOperationSupport(outputGate);
    }
}

bool PacketSchedulerBase::canPullSomePacket(cGate *gate) const
{
    for (int i = 0; i < (int)inputGates.size(); i++) {
        auto inputProvider = providers[i];
        if (inputProvider->canPullSomePacket(inputGates[i]->getPathStartGate()))
            return true;
    }
    return false;
}

Packet *PacketSchedulerBase::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    int index = schedulePacket();
    if (index < 0 || static_cast<unsigned int>(index) >= inputGates.size())
        throw cRuntimeError("Scheduled packet from invalid input gate: %d", index);
    auto packet = providers[index]->pullPacket(inputGates[index]->getPathStartGate());
    take(packet);
    EV_INFO << "Scheduling packet " << packet->getName() << ".\n";
    animateSend(packet, outputGate);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    emit(packetPulledSignal, packet);
    return packet;
}

void PacketSchedulerBase::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

} // namespace queueing
} // namespace inet

