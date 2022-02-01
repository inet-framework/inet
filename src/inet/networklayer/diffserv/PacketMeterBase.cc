//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/diffserv/PacketMeterBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

void PacketMeterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_LAST) {
        checkPacketOperationSupport(inputGate);
    }
}

void PacketMeterBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketMeterBase::handleCanPushPacketChanged(cGate *gate)
{
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PacketMeterBase::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, gate, successful);
}

} // namespace inet

