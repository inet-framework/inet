// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mac/aggregation/HtMpduAggregationPolicy.h"

#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregation.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"

namespace inet {
namespace ieee80211 {

Define_Module(HtMpduAggregationPolicy);

std::vector<Packet *> HtMpduAggregationPolicy::selectFrames(const std::vector<Packet *>& candidates,
        const MpduAggregationLimits& limits) const
{
    auto mode = dynamic_cast<const physicallayer::Ieee80211HtMode *>(limits.mode);
    if (!mode || mode->getDataMode()->getBandwidth() != MHz(20) || mode->isHtShortGuardInterval() ||
            mode->getPreambleMode()->getPreambleFormat() != physicallayer::Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED)
        return {};
    std::vector<Packet *> selected;
    MpduAggregation builder;
    std::set<int> sequences;
    for (auto packet : candidates) {
        auto header = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
        if (!header || header->getType() != ST_DATA_WITH_QOS || header->getReceiverAddress().isMulticast() ||
                header->getReceiverAddress() != limits.receiver || header->getTransmitterAddress() != limits.transmitter ||
                header->getTid() != limits.tid || header->getAMsduPresent() || header->getMoreFragments() ||
                header->getFragmentNumber() != 0 || !header->getSequenceNumber().isSpecified())
            break;
        int distance = OriginatorBlockAckAgreement::computeSequenceDistance(limits.windowStart, header->getSequenceNumber());
        if (distance >= limits.windowSize || !sequences.insert(header->getSequenceNumber().get()).second ||
                selected.size() >= size_t(std::min(64, limits.windowSize)))
            break;
        selected.push_back(packet);
        B length = builder.computeAggregateLength(selected, mode, limits.minimumMpduStartSpacing);
        if (length == B(0) || length > limits.maxLength || !mode->isPpduDurationValid(length)) {
            selected.pop_back();
            break;
        }
    }
    return selected;
}

} // namespace ieee80211
} // namespace inet
