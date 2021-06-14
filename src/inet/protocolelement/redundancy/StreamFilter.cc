//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/redundancy/StreamFilter.h"

#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamFilter);

void StreamFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        streamNameFilter.setPattern(par("streamNameFilter"), false, true, true);
}

bool StreamFilter::matchesPacket(const Packet *packet) const
{
    auto streamReq = packet->findTag<StreamReq>();
    if (streamReq != nullptr) {
        auto streamName = streamReq->getStreamName();
        cMatchableString matchableString(streamName);
        return const_cast<cMatchExpression *>(&streamNameFilter)->matches(&matchableString);
    }
    return false;
}

} // namespace inet

