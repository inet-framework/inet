//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PassivePacketSourceBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

void PassivePacketSourceBase::initialize(int stage)
{
    PacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        collector = findConnectedModule<IActivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

Packet *PassivePacketSourceBase::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    ASSERT(gate->getOwnerModule() == this);
    ASSERT(canPullPacket(gate));
    auto packet = handlePullPacket(gate);
    ASSERT(packet != nullptr);
    return packet;
}

} // namespace queueing
} // namespace inet

