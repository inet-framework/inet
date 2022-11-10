//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/common/BackPressureBarrier.h"

namespace inet {
namespace queueing {

Define_Module(BackPressureBarrier);

cGate *BackPressureBarrier::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

Packet *BackPressureBarrier::canPullPacket(cGate *gate) const
{
    auto packet = provider->canPullPacket(inputGate->getPathStartGate());
    if (packet == nullptr)
        throw cRuntimeError("Cannot pull packet from the other side of the backpressure barrier");
    return packet;
}

} // namespace queueing
} // namespace inet

