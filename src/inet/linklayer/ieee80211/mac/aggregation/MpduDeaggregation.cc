//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MpduDeaggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduDeaggregation);

std::vector<Packet *> *MpduDeaggregation::deaggregateFrame(Packet *aggregatedFrame)
{
    EV_DEBUG << "Deaggregating A-MPDU " << *aggregatedFrame << " into multiple packets.\n";
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    int paddingLength = 0;
    cStringTokenizer tokenizer(aggregatedFrame->getName(), "+");
    while (aggregatedFrame->getDataLength() > b(0)) {
        aggregatedFrame->setFrontOffset(aggregatedFrame->getFrontOffset() + B(paddingLength == 4 ? 0 : paddingLength));
        const auto& mpduSubframeHeader = aggregatedFrame->popAtFront<Ieee80211MpduSubframeHeader>();
        const auto& mpdu = aggregatedFrame->peekDataAt(b(0), B(mpduSubframeHeader->getLength()));
        paddingLength = 4 - B(mpduSubframeHeader->getChunkLength() + mpdu->getChunkLength()).get() % 4;
        aggregatedFrame->setFrontOffset(aggregatedFrame->getFrontOffset() + mpdu->getChunkLength());
        auto frame = new Packet();
        frame->setName(tokenizer.nextToken());
        frame->insertAtBack(mpdu);
        frame->getRegionTags().copyTags(aggregatedFrame->getRegionTags(), aggregatedFrame->getFrontOffset() - frame->getDataLength(), frame->getFrontOffset(), frame->getDataLength());
        EV_TRACE << "Created " << *frame << " from A-MPDU.\n";
        // TODO check CRC?
        frames->push_back(frame);
    }
    delete aggregatedFrame;
    EV_TRACE << "Created " << frames->size() << " packets from A-MPDU.\n";
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

