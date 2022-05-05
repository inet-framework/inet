//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduAggregation);

Packet *MpduAggregation::aggregateFrames(std::vector<Packet *> *frames)
{
    EV_DEBUG << "Aggregating " << frames->size() << " packets into A-MPDU.\n";
    auto aggregatedFrame = new Packet();
    std::string aggregatedName;
    for (int i = 0; i < (int)frames->size(); i++) {
        auto mpduSubframeHeader = makeShared<Ieee80211MpduSubframeHeader>();
        auto frame = frames->at(i);
        auto mpdu = frame->peekAll();
        mpduSubframeHeader->setLength(B(mpdu->getChunkLength()).get());
        aggregatedFrame->insertAtBack(mpduSubframeHeader);
        aggregatedFrame->insertAtBack(mpdu);
        aggregatedFrame->getRegionTags().copyTags(frame->getRegionTags(), B(0), aggregatedFrame->getFrontOffset() - frame->getDataLength(), frame->getDataLength());
        int paddingLength = 4 - B(mpduSubframeHeader->getChunkLength() + mpdu->getChunkLength()).get() % 4;
        if (i != (int)frames->size() - 1 && paddingLength != 4) {
            auto padding = makeShared<ByteCountChunk>(B(paddingLength));
            aggregatedFrame->insertAtBack(padding);
        }
        if (i != 0)
            aggregatedName.append("+");
        aggregatedName.append(frame->getName());
        delete frame;
    }
    aggregatedFrame->setName(aggregatedName.c_str());
    EV_TRACE << "Created A-MPDU " << *aggregatedFrame << ".\n";
    return aggregatedFrame;
}

} /* namespace ieee80211 */
} /* namespace inet */

