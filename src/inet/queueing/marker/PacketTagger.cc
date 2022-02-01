//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/marker/PacketTagger.h"

namespace inet {
namespace queueing {

Define_Module(PacketTagger);

PacketTagger::~PacketTagger()
{
    delete packetFilterFunction;
}

void PacketTagger::initialize(int stage)
{
    PacketTaggerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        packetFilterFunction = createFilterFunction(par("filterClass"));
}

IPacketFilterFunction *PacketTagger::createFilterFunction(const char *filterClass) const
{
    return check_and_cast<IPacketFilterFunction *>(createOne(filterClass));
}

void PacketTagger::markPacket(Packet *packet)
{
    if (packetFilterFunction->matchesPacket(packet))
        PacketTaggerBase::markPacket(packet);
}

} // namespace queueing
} // namespace inet

