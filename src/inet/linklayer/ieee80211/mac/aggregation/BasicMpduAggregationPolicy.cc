//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/BasicMpduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicMpduAggregationPolicy);

void BasicMpduAggregationPolicy::initialize()
{
    maxMpduCount = par("maxMpduCount");
    if (maxMpduCount < 1 || maxMpduCount > 64)
        throw cRuntimeError("maxMpduCount must be in 1..64 for immediate Compressed Block Ack");
}

std::vector<Packet *> *BasicMpduAggregationPolicy::computeAggregateFrames(std::vector<Packet *> *frames)
{
    Enter_Method("computeAggregateFrames");
    if (frames->empty())
        return nullptr;
    auto first = dynamicPtrCast<const Ieee80211DataHeader>(frames->front()->peekAtFront<Ieee80211MacHeader>());
    if (!first || first->getType() != ST_DATA_WITH_QOS || first->getReceiverAddress().isMulticast())
        return nullptr;
    auto selected = new std::vector<Packet *>();
    for (auto frame : *frames) {
        auto header = dynamicPtrCast<const Ieee80211DataHeader>(frame->peekAtFront<Ieee80211MacHeader>());
        // IEEE Std 802.11-2024, 10.12.1: EOSP agrees across the whole
        // A-MPDU; same-TID bits8--15 agree, while A-MSDU Present may differ.
        if (!header || header->getType() != ST_DATA_WITH_QOS || header->getMoreFragments() || header->getFragmentNumber() != 0
                || header->getReceiverAddress() != first->getReceiverAddress() || header->getTransmitterAddress() != first->getTransmitterAddress()
                || header->getTid() != first->getTid() || header->getEosp() != first->getEosp()
                || header->getQosControlHigh() != first->getQosControlHigh())
            break;
        selected->push_back(frame);
        if (selected->size() == size_t(maxMpduCount))
            break;
    }
    if (selected->empty()) { delete selected; return nullptr; }
    return selected;
}

} /* namespace ieee80211 */
} /* namespace inet */

