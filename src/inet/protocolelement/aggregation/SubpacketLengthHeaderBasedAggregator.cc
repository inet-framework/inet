//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    aggregatedPacket->getRegionTags().copyTags(packet->getRegionTags(), packet->getFrontOffset(), aggregatedPacket->getBackOffset() - packet->getDataLength(), packet->getDataLength());
}

} // namespace inet

