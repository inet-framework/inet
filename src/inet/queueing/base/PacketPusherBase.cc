//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketPusherBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketPusherBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        producer.reference(inputGate, false); // TODO: lookup with accepting anything ignoring the lookup arguments
        consumer.reference(outputGate, false); // TODO: lookup with accepting anything ignoring the lookup arguments
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketPusherBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

bool PacketPusherBase::canPushSomePacket(const cGate *gate) const
{
    return consumer.canPushSomePacket();
}

bool PacketPusherBase::canPushPacket(Packet *packet, const cGate *gate) const
{
    return consumer.canPushPacket(packet);
}

void PacketPusherBase::pushPacket(Packet *packet, const cGate *gate)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::pushPacketStart(Packet *packet, const cGate *gate, bps datarate)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::pushPacketEnd(Packet *packet, const cGate *gate)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer.handleCanPushPacketChanged();
}

void PacketPusherBase::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer.handlePushPacketProcessed(packet, successful);
}

} // namespace queueing
} // namespace inet

