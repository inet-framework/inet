//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/filter/PacketFilter.h"

namespace inet {
namespace queueing {

Define_Module(PacketFilter);

void PacketFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        packetFilterFunction = createFilterFunction(par("filterClass"));
}

IPacketFilterFunction *PacketFilter::createFilterFunction(const char *filterClass) const
{
    return check_and_cast<IPacketFilterFunction *>(createOne(filterClass));
}

bool PacketFilter::matchesPacket(const Packet *packet) const
{
    return packetFilterFunction->matchesPacket(packet);
}

} // namespace queueing
} // namespace inet

