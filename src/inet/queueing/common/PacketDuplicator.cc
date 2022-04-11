//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/PacketDuplicator.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(PacketDuplicator);

void PacketDuplicator::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    int numDuplicates = par("numDuplicates");
    for (int i = 0; i < numDuplicates; i++) {
        EV_INFO << "Forwarding duplicate packet" << EV_FIELD(packet) << EV_ENDL;
        auto duplicate = packet->dup();
        pushOrSendPacket(duplicate, outputGate, consumer);
    }
    EV_INFO << "Forwarding original packet" << EV_FIELD(packet) << EV_ENDL;
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void PacketDuplicator::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PacketDuplicator::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    producer->handlePushPacketProcessed(packet, gate, successful);
}

} // namespace queueing
} // namespace inet

