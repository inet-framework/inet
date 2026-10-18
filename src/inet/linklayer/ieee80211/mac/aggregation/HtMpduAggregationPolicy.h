// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_HTMPDUAGGREGATIONPOLICY_H
#define __INET_HTMPDUAGGREGATIONPOLICY_H

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregationPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API HtMpduAggregationPolicy : public SimpleModule, public IMpduAggregationPolicy
{
  public:
    virtual bool isEnabled() const override { return true; }
    // The legacy consuming path has no negotiated context and cannot enable HT aggregation.
    virtual std::vector<Packet *> *computeAggregateFrames(std::vector<Packet *> *frames) override { return nullptr; }
    virtual std::vector<Packet *> selectFrames(const std::vector<Packet *>& candidates, const MpduAggregationLimits& limits) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
