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
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
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

bool PacketPusherBase::canPushSomePacket(cGate *gate) const
{
    return consumer->canPushSomePacket(outputGate->getPathEndGate());
}

bool PacketPusherBase::canPushPacket(Packet *packet, cGate *gate) const
{
    return consumer->canPushPacket(packet, outputGate->getPathEndGate());
}

void PacketPusherBase::pushPacket(Packet *packet, cGate *gate)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::pushPacketEnd(Packet *packet, cGate *gate)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPusherBase::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PacketPusherBase::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

} // namespace queueing
} // namespace inet

