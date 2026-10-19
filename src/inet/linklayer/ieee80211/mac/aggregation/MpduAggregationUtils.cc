// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregationUtils.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

MpduLayout inspectMpduAggregate(const Packet *packet, B offset, B length,
        const std::function<bool(B, B)>& delimiterSuccessful)
{
    MpduLayout layout;
    if (offset < B(0) || length < B(0) || offset + length > packet->getDataLength()) {
        layout.complete = false;
        return layout;
    }
    B position(0);
    // IEEE Std 802.11-2024, 9.7.1: delimiter boundaries are four octets apart after loss of continuity.
    while (position < length) {
        if (length - position < B(4)) {
            layout.ranges.push_back({MpduRangeKind::DELIMITER, position, length - position, false});
            layout.complete = false;
            break;
        }
        // Recovery can enter a typed MPDU. Interpret its candidate delimiter bytes explicitly.
        const auto& delimiter = packet->peekDataAt<Ieee80211MpduSubframeHeader>(offset + position, B(4),
                Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED |
                Chunk::PF_ALLOW_REINTERPRETATION);
        bool valid = delimiter->isCorrect() && delimiter->isComplete() && delimiter->isProperlyRepresented() &&
                delimiter->getLength() >= 0 && delimiter->getLength() <= 4095 &&
                B(delimiter->getLength()) <= length - position - B(4) &&
                (!delimiterSuccessful || delimiterSuccessful(position, B(4)));
        layout.ranges.push_back({MpduRangeKind::DELIMITER, position, B(4), valid});
        position += B(4);
        if (!valid) {
            layout.complete = false;
            continue;
        }
        B memberLength(delimiter->getLength());
        if (memberLength == B(0))
            continue;
        layout.ranges.push_back({MpduRangeKind::MPDU, position, memberLength, true});
        position += memberLength;
        if (position == length)
            break;
        B padding((4 - memberLength.get<B>() % 4) % 4);
        if (padding > length - position) {
            layout.ranges.push_back({MpduRangeKind::DELIMITER, position, length - position, false});
            layout.complete = false;
            break;
        }
        position += padding;
        if (position == length) {
            layout.ranges.push_back({MpduRangeKind::DELIMITER, position - padding, padding, false});
            layout.complete = false; // HT has no final alignment padding.
        }
    }
    return layout;
}

} // namespace ieee80211
} // namespace inet
