//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/aggregation/policy/LengthBasedAggregatorPolicy.h"

namespace inet {

Define_Module(LengthBasedAggregatorPolicy);

void LengthBasedAggregatorPolicy::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        minNumSubpackets = par("minNumSubpackets");
        maxNumSubpackets = par("maxNumSubpackets");
        minAggregatedLength = b(par("minAggregatedLength"));
        maxAggregatedLength = b(par("maxAggregatedLength"));
    }
}

bool LengthBasedAggregatorPolicy::isAggregatablePacket(Packet *aggregatedPacket, std::vector<Packet *>& aggregatedSubpackets, Packet *newSubpacket)
{
//    b aggregatedLength = aggregatedPacket == nullptr ? b(0) : aggregatedPacket->getTotalLength();
    return (int)aggregatedSubpackets.size() < minNumSubpackets ||
           aggregatedPacket->getTotalLength() < minAggregatedLength ||
           ((int)aggregatedSubpackets.size() + 1 <= maxNumSubpackets &&
            aggregatedPacket->getTotalLength() + newSubpacket->getTotalLength() <= maxAggregatedLength);
}

} // namespace inet

