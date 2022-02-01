//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketPullerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

void PacketPullerBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        collector = findConnectedModule<IActivePacketSink>(outputGate);
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

bool PacketPullerBase::canPullSomePacket(cGate *gate) const
{
    return provider->canPullSomePacket(inputGate->getPathStartGate());
}

Packet *PacketPullerBase::canPullPacket(cGate *gate) const
{
    return provider->canPullPacket(inputGate->getPathStartGate());
}

Packet *PacketPullerBase::pullPacket(cGate *gate)
{
    throw cRuntimeError("Invalid operation");
}

Packet *PacketPullerBase::pullPacketStart(cGate *gate, bps datarate)
{
    throw cRuntimeError("Invalid operation");
}

Packet *PacketPullerBase::pullPacketEnd(cGate *gate)
{
    throw cRuntimeError("Invalid operation");
}

Packet *PacketPullerBase::pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    throw cRuntimeError("Invalid operation");
}

void PacketPullerBase::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collector != nullptr)
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
}

void PacketPullerBase::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    if (collector != nullptr)
        collector->handlePullPacketProcessed(packet, outputGate->getPathEndGate(), successful);
}

} // namespace queueing
} // namespace inet

