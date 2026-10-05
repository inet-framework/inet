//
// Copyright (C) 2026 OpenSim Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211RATECONTEXT_H
#define __INET_IEEE80211RATECONTEXT_H

#include "inet/linklayer/ieee80211/mib/Ieee80211RateSet.h"

namespace inet {
namespace ieee80211 {

struct INET_API BssRateContextRef
{
    enum Kind { ACTIVE = 0, TARGET = 1, NONE = 2 };
    Kind kind = NONE;
    MacAddress bssid;
    uint64_t transactionId = 0;
    uint64_t generation = 0;

    bool operator==(const BssRateContextRef& other) const
    {
        return kind == other.kind && bssid == other.bssid &&
                transactionId == other.transactionId && generation == other.generation;
    }
    bool operator!=(const BssRateContextRef& other) const { return !(*this == other); }
};

struct INET_API RateContextSnapshot
{
    bool known = false;
    BssRateContextRef context;
    MacAddress localAddress;
    MacAddress peerAddress;
    uint64_t generation = 0;
    Ieee80211RateSetState localRates;
    Ieee80211RateSetState bssRates;
    Ieee80211RateSetState peerRates;
};

} // namespace ieee80211
} // namespace inet

#endif
