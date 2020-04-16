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

#include "inet/queueing/common/RateTag_m.h"
#include "inet/queueing/filter/RateLimiter.h"

namespace inet {
namespace queueing {

Define_Module(RateLimiter);

void RateLimiter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        maxDatarate = bps(par("maxDatarate"));
        maxPacketrate = par("maxPacketrate");
    }
}

bool RateLimiter::matchesPacket(const Packet *packet) const
{
    auto rateTag = packet->getTag<RateTag>();
    double p = 0;
    if (!std::isnan(maxDatarate.get()) && rateTag->getDatarate() > maxDatarate)
        p = std::max(p, unit((rateTag->getDatarate() - maxDatarate) / rateTag->getDatarate()).get());
    else if (!std::isnan(maxPacketrate) && rateTag->getPacketrate() > maxPacketrate)
        p = std::max(p, (rateTag->getPacketrate() - maxPacketrate) / rateTag->getPacketrate());
    return p == 0 || dblrand() >= p;
}

} // namespace queueing
} // namespace inet

