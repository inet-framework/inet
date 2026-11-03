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
    // IEEE Std 802.11-2024, 9.7: bound each delimiter, MPDU and inter-MPDU
    // padding before consuming any part of the aggregate.
    auto offset = b(0);
    auto end = aggregatedFrame->getDataLength();
    if (end == b(0))
        return nullptr;
    while (offset < end) {
        if (end - offset < LENGTH_A_MPDU_SUBFRAME_HEADER)
            return nullptr;
        const auto& delimiter = aggregatedFrame->peekDataAt<Ieee80211MpduSubframeHeader>(offset, LENGTH_A_MPDU_SUBFRAME_HEADER);
        int length = delimiter->getLength();
        if (length < 0 || length > 4095 || B(length) > end - offset - LENGTH_A_MPDU_SUBFRAME_HEADER)
            return nullptr;
        offset += LENGTH_A_MPDU_SUBFRAME_HEADER + B(length);
        if (offset == end)
            break;
        auto padding = B((4 - length % 4) % 4);
        if (end - offset < padding + LENGTH_A_MPDU_SUBFRAME_HEADER)
            return nullptr;
        offset += padding;
    }
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    int paddingLength = 0;
    cStringTokenizer tokenizer(aggregatedFrame->getName(), "+");
    while (aggregatedFrame->getDataLength() > b(0)) {
        aggregatedFrame->setFrontOffset(aggregatedFrame->getFrontOffset() + B(paddingLength == 4 ? 0 : paddingLength));
        const auto& mpduSubframeHeader = aggregatedFrame->popAtFront<Ieee80211MpduSubframeHeader>();
        // A zero-length delimiter is padding, not an empty MPDU.
        if (mpduSubframeHeader->getLength() == 0) {
            paddingLength = 0;
            continue;
        }
        const auto& mpdu = aggregatedFrame->peekDataAt(b(0), B(mpduSubframeHeader->getLength()));
        paddingLength = 4 - (mpduSubframeHeader->getChunkLength() + mpdu->getChunkLength()).get<B>() % 4;
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

