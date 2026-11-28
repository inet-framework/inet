//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMPDUAGGREGATION_H
#define __INET_IMPDUAGGREGATION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace physicallayer {
class IIeee80211Mode;
}
namespace ieee80211 {

class INET_API IMpduAggregation
{
  public:
    virtual ~IMpduAggregation() {}

    virtual Packet *aggregateFrames(std::vector<Packet *> *frames) = 0;

    // These HT operations borrow the sources. Zero length or nullptr means refusal without source changes.
    // Spacing is the receiver's encoded value (0..7). The byte limit includes delimiters and padding.
    virtual B computeAggregateLength(const std::vector<Packet *>& frames, const physicallayer::IIeee80211Mode *mode,
            int minimumMpduStartSpacing) const = 0;
    virtual Packet *aggregateFrames(const std::vector<Packet *>& frames, const physicallayer::IIeee80211Mode *mode,
            int minimumMpduStartSpacing, B maxAggregateLength) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

