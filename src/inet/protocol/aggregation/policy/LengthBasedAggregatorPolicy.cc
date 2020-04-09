//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/protocol/aggregation/policy/LengthBasedAggregatorPolicy.h"

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
    return aggregatedSubpackets.size() < minNumSubpackets ||
           aggregatedPacket->getTotalLength() < minAggregatedLength ||
           (aggregatedSubpackets.size() + 1 <= maxNumSubpackets &&
            aggregatedPacket->getTotalLength() + newSubpacket->getTotalLength() <= maxAggregatedLength);
}

} // namespace inet

