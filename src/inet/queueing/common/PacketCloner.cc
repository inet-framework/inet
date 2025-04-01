//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketCloner.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PacketCloner);

void PacketCloner::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer.reference(inputGate, false);
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            outputGates.push_back(outputGate);
            PassivePacketSinkRef consumer;
            consumer.reference(outputGate, true);
            consumers.push_back(consumer);
        }
    }
}

void PacketCloner::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketCloner::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    int numGates = outputGates.size();
    handlePacketProcessed(packet);
    for (int i = 0; i < numGates; i++) {
        EV_INFO << "Cloning packet" << EV_FIELD(packet) << EV_ENDL;
        pushOrSendPacket(i == numGates - 1 ? packet : packet->dup(), outputGates[i], consumers[i]);
    }
}

void PacketCloner::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer.handleCanPushPacketChanged();
}

void PacketCloner::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    producer.handlePushPacketProcessed(packet, successful);
}

} // namespace queueing
} // namespace inet

