//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/sink/FullPacketSink.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(FullPacketSink);

void FullPacketSink::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(inputGate);
}

void FullPacketSink::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
}

void FullPacketSink::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
}

} // namespace queueing
} // namespace inet

