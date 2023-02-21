//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/filter/ContentBasedFilter.h"

namespace inet {
namespace queueing {

Define_Module(ContentBasedFilter);

void ContentBasedFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        filter.setExpression(par("packetFilter").objectValue());
}

cGate *ContentBasedFilter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

bool ContentBasedFilter::matchesPacket(const Packet *packet) const
{
    return filter.matches(packet);
}

} // namespace queueing
} // namespace inet

