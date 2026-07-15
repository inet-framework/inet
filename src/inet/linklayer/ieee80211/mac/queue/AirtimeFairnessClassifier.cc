//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessClassifier.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeFairnessClassifier);

void AirtimeFairnessClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        WATCH_EXPR("numStations", (int)addressToIndex.size());
}

MacAddress AirtimeFairnessClassifier::getReceiverAddress(Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    return header->getReceiverAddress();
}

int AirtimeFairnessClassifier::classifyPacket(Packet *packet)
{
    MacAddress address = getReceiverAddress(packet);
    auto it = addressToIndex.find(address);
    if (it != addressToIndex.end())
        return it->second;
    // first frame for this receiver -> assign the next free branch
    int index = (int)addressToIndex.size();
    addressToIndex[address] = index;
    return index;
}

} // namespace ieee80211
} // namespace inet
