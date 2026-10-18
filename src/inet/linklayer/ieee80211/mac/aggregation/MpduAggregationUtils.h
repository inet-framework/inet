// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_MPDUAGGREGATIONUTILS_H
#define __INET_MPDUAGGREGATIONUTILS_H

#include <functional>

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

enum class MpduRangeKind { DELIMITER, MPDU };

struct INET_API MpduRange
{
    MpduRangeKind kind;
    B offset;
    B length;
    // Delimiter validity includes header flags and member bounds. MPDU validity requires a complete range.
    bool valid;
};

struct INET_API MpduLayout
{
    std::vector<MpduRange> ranges;
    bool complete = true;
};

// Offsets are relative to the PSDU, without the PHY header or PHY padding.
INET_API MpduLayout inspectMpduAggregate(const Packet *packet, B offset, B length,
        const std::function<bool(B, B)>& delimiterSuccessful = {});

} // namespace ieee80211
} // namespace inet

#endif
