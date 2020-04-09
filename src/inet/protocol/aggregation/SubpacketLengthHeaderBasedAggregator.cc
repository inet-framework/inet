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

#include "inet/protocol/aggregation/header/SubpacketLengthHeader_m.h"
#include "inet/protocol/aggregation/SubpacketLengthHeaderBasedAggregator.h"

namespace inet {

Define_Module(SubpacketLengthHeaderBasedAggregator);

void SubpacketLengthHeaderBasedAggregator::continueAggregation(Packet *packet)
{
    AggregatorBase::continueAggregation(packet);
    auto subpacketHeader = makeShared<SubpacketLengthHeader>();
    subpacketHeader->setLengthField(packet->getDataLength());
    aggregatedPacket->insertAtBack(subpacketHeader);
    aggregatedPacket->insertAtBack(packet->peekData());
}

} // namespace inet

