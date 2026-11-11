// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IMESHLINKMETRIC_H
#define INET_IMESHLINKMETRIC_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet::ieee80211 {

struct Ieee80211RateSetState;

/** Outgoing directional cost; the selected metric identifier belongs to the MIB. */
class INET_API IMeshLinkMetric
{
  public:
    virtual ~IMeshLinkMetric() = default;
    virtual uint32_t getMetric(MacAddress peer) = 0;
    virtual void resetPeer(MacAddress peer) = 0;
    // Aggregation eligibility/limits changed without a PHY/error-estimator reset.
    virtual void resetAggregation(MacAddress peer) = 0;
    virtual bool supportsPeerRates(MacAddress peer, const Ieee80211RateSetState& rates) = 0;
};

} // namespace inet::ieee80211
#endif
