//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SINGLEPROTECTIONMECHANISM_H
#define __INET_SINGLEPROTECTIONMECHANISM_H
#include "inet/common/SimpleModule.h"
#include "inet/linklayer/ieee80211/mac/common/TxopExchangePlan.h"
namespace inet {
namespace ieee80211 {
class INET_API SingleProtectionMechanism : public SimpleModule
{
  public:
    virtual simtime_t computeDurationField(const ITransmitStep *step, const TxopExchangePlan& plan,
            const TxopExchangePlan *continuation) const;
};
} // namespace ieee80211
} // namespace inet
#endif
