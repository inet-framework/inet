//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketServerBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void PacketServerBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        provider.reference(inputGate, false);
        outputGate = gate("out");
        consumer.reference(outputGate, false);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
    }
}

void PacketServerBase::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
}

void PacketServerBase::handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
}

} // namespace queueing
} // namespace inet

