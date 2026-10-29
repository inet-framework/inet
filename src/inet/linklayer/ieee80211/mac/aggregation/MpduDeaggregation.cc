//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MpduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregationUtils.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduDeaggregation);

std::vector<Packet *> *MpduDeaggregation::deaggregateFrame(Packet *aggregatedFrame)
{
    EV_DEBUG << "Deaggregating A-MPDU " << *aggregatedFrame << " into multiple packets.\n";
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    auto layout = inspectMpduAggregate(aggregatedFrame, B(0), B(aggregatedFrame->getDataLength()));
    // An incomplete layout can contain bounded recovered members. Extraction does not establish MAC acceptance.
    // The caller must check delimiter validity, PHY outcomes, member FCS, and MAC conditions before acceptance.
    cStringTokenizer tokenizer(aggregatedFrame->getName(), "+");
    for (const auto& range : layout.ranges) {
        if (range.kind != MpduRangeKind::MPDU)
            continue;
        const auto& mpdu = aggregatedFrame->peekDataAt(range.offset, range.length);
        auto frame = new Packet();
        frame->setName(tokenizer.hasMoreTokens() ? tokenizer.nextToken() : aggregatedFrame->getName());
        frame->insertAtBack(mpdu);
        frame->getRegionTags().copyTags(aggregatedFrame->getRegionTags(), aggregatedFrame->getFrontOffset() + range.offset, frame->getFrontOffset(), range.length);
        EV_TRACE << "Created " << *frame << " from A-MPDU.\n";
        frames->push_back(frame);
    }
    delete aggregatedFrame;
    EV_TRACE << "Created " << frames->size() << " packets from A-MPDU.\n";
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

