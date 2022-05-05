//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/aggregation/SubpacketLengthHeaderBasedDeaggregator.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/aggregation/header/SubpacketLengthHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"

namespace inet {

Define_Module(SubpacketLengthHeaderBasedDeaggregator);

void SubpacketLengthHeaderBasedDeaggregator::initialize(int stage)
{
    DeaggregatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(AccessoryProtocol::aggregation, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::aggregation, nullptr, outputGate);
    }
}

std::vector<Packet *> SubpacketLengthHeaderBasedDeaggregator::deaggregatePacket(Packet *packet)
{
    std::vector<Packet *> subpackets;
    cStringTokenizer tokenizer(packet->getName(), "+");
    while (packet->getDataLength() > b(0)) {
        auto subpacketHeader = packet->popAtFront<SubpacketLengthHeader>();
        auto subpacketData = packet->popAtFront(subpacketHeader->getLengthField());
        auto subpacketName = tokenizer.nextToken();
        auto subpacket = new Packet(subpacketName, subpacketData);
        subpacket->getRegionTags().copyTags(packet->getRegionTags(), packet->getFrontOffset() - subpacket->getDataLength(), B(0), subpacket->getDataLength());
        subpackets.push_back(subpacket);
    }
    return subpackets;
}

} // namespace inet

