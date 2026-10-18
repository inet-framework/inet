//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduAggregation);

Packet *MpduAggregation::aggregateFrames(std::vector<Packet *> *frames)
{
    auto aggregate = buildAggregate(*frames, B(0));
    for (auto frame : *frames)
        delete frame;
    return aggregate;
}

B MpduAggregation::computeAggregateLength(const std::vector<Packet *>& frames, const physicallayer::IIeee80211Mode *mode,
        int minimumMpduStartSpacing) const
{
    auto htMode = dynamic_cast<const physicallayer::Ieee80211HtMode *>(mode);
    if (htMode == nullptr)
        return B(0);
    B minimumDistance = htMode->getMinimumMpduStartDistance(minimumMpduStartSpacing);
    B length(0);
    for (size_t i = 0; i < frames.size(); i++) {
        auto bits = frames[i]->getDataLength();
        // IEEE Std 802.11-2024, 9.7.1: each nonzero HT member has a 12-bit octet length.
        if (bits <= b(0) || bits.get<b>() % 8 != 0 || bits > B(4095))
            return B(0);
        B subframeLength = B(4) + B(bits);
        if (i + 1 != frames.size())
            subframeLength = B((std::max(subframeLength, minimumDistance).get<B>() + 3) / 4 * 4);
        length += subframeLength;
        if (length > B(65535))
            return B(0);
    }
    return length;
}

Packet *MpduAggregation::aggregateFrames(const std::vector<Packet *>& frames, const physicallayer::IIeee80211Mode *mode,
        int minimumMpduStartSpacing, B maxAggregateLength)
{
    if (maxAggregateLength < B(0))
        throw cRuntimeError("Maximum aggregate length must be nonnegative");
    B length = computeAggregateLength(frames, mode, minimumMpduStartSpacing);
    if (length == B(0) || length > maxAggregateLength)
        return nullptr;
    auto htMode = check_and_cast<const physicallayer::Ieee80211HtMode *>(mode);
    if (!htMode->isPpduDurationValid(length))
        return nullptr;
    return buildAggregate(frames, htMode->getMinimumMpduStartDistance(minimumMpduStartSpacing));
}

Packet *MpduAggregation::buildAggregate(const std::vector<Packet *>& frames, B minimumStartDistance) const
{
    EV_DEBUG << "Aggregating " << frames.size() << " packets into A-MPDU.\n";
    auto aggregatedFrame = std::make_unique<Packet>();
    std::string aggregatedName;
    for (size_t i = 0; i < frames.size(); i++) {
        auto mpduSubframeHeader = makeShared<Ieee80211MpduSubframeHeader>();
        auto frame = frames.at(i);
        auto mpdu = frame->peekData();
        mpduSubframeHeader->setLength(mpdu->getChunkLength().get<B>());
        aggregatedFrame->insertAtBack(mpduSubframeHeader);
        aggregatedFrame->insertAtBack(mpdu);
        aggregatedFrame->getRegionTags().copyTags(frame->getRegionTags(), frame->getFrontOffset(),
                aggregatedFrame->getBackOffset() - frame->getDataLength(), frame->getDataLength());
        if (i + 1 != frames.size()) {
            // IEEE Std 802.11-2024, 9.7.1 and 10.12.3: alignment precedes zero-length delimiters.
            B subframeLength = B(4) + B(mpdu->getChunkLength());
            B alignedLength((subframeLength.get<B>() + 3) / 4 * 4);
            B paddingLength = alignedLength - subframeLength;
            if (paddingLength != B(0))
                aggregatedFrame->insertAtBack(makeShared<ByteCountChunk>(paddingLength, 0));
            while (alignedLength < minimumStartDistance) {
                aggregatedFrame->insertAtBack(makeShared<Ieee80211MpduSubframeHeader>());
                alignedLength += B(4);
            }
        }
        if (i != 0)
            aggregatedName.append("+");
        aggregatedName.append(frame->getName());
    }
    aggregatedFrame->setName(aggregatedName.c_str());
    EV_TRACE << "Created A-MPDU " << *aggregatedFrame << ".\n";
    return aggregatedFrame.release();
}

} /* namespace ieee80211 */
} /* namespace inet */

