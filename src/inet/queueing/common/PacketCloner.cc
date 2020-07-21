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
#include "inet/queueing/common/PacketCloner.h"

namespace inet {
namespace queueing {

Define_Module(PacketCloner);

void PacketCloner::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            auto consumer = getConnectedModule<IPassivePacketSink>(outputGate);
            outputGates.push_back(outputGate);
            consumers.push_back(consumer);
        }
    }
}

void PacketCloner::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketCloner::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    int numGates = outputGates.size();
    for (int i = 0; i < numGates; i++) {
        EV_INFO << "Cloning packet " << packet->getName() << "." << endl;
        pushOrSendPacket(i == numGates - 1 ? packet : packet->dup(), outputGates[i], consumers[i]);
    }
    numProcessedPackets++;
    processedTotalLength += packet->getTotalLength();
    updateDisplayString();
}

void PacketCloner::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PacketCloner::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    producer->handlePushPacketProcessed(packet, gate, successful);
}

} // namespace queueing
} // namespace inet

