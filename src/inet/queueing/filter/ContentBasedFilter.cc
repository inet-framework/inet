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

bool ContentBasedFilter::matchesPacket(const Packet *packet) const
{
    return filter.matches(packet);
}

} // namespace queueing
} // namespace inet

