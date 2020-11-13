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

#include "inet/protocolelement/aggregation/SubpacketLengthHeaderBasedAggregator.h"

#include "inet/protocolelement/aggregation/header/SubpacketLengthHeader_m.h"

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

