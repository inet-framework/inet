//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/BackPressureBarrier.h"

namespace inet {
namespace queueing {

Define_Module(BackPressureBarrier);

Packet *BackPressureBarrier::canPullPacket(const cGate *gate) const
{
    auto packet = provider.canPullPacket();
    if (packet == nullptr)
        throw cRuntimeError("Cannot pull packet from the other side of the backpressure barrier");
    return packet;
}

} // namespace queueing
} // namespace inet

