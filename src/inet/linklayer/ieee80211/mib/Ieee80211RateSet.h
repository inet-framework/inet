//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RATESET_H
#define __INET_IEEE80211RATESET_H

#include <cmath>
#include <set>

#include "inet/common/Units.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

/**
 * A rate set preserves absent information and a known empty set.
 * Legacy rates use bps units. HT rates use explicit MCS indexes.
 */
struct INET_API Ieee80211RateSet
{
    bool known = false;
    std::set<bps> legacyRates;
    std::set<int> htMcs;

    bool operator==(const Ieee80211RateSet& other) const
    {
        return known == other.known && legacyRates == other.legacyRates && htMcs == other.htMcs;
    }

    bool operator!=(const Ieee80211RateSet& other) const { return !(*this == other); }
};

/**
 * Supported, basic, and operational rate information for one owner.
 */
struct INET_API Ieee80211RateSetState
{
    Ieee80211RateSet supported;
    Ieee80211RateSet basic;
    Ieee80211RateSet operational;

    bool operator==(const Ieee80211RateSetState& other) const
    {
        return supported == other.supported && basic == other.basic && operational == other.operational;
    }

    bool operator!=(const Ieee80211RateSetState& other) const { return !(*this == other); }
};

inline void validateIeee80211RateSet(const Ieee80211RateSet& rateSet, const char *name)
{
    if (!rateSet.known && (!rateSet.legacyRates.empty() || !rateSet.htMcs.empty()))
        throw cRuntimeError("Unknown %s cannot contain rates", name);
    for (auto rate : rateSet.legacyRates) {
        if (!std::isfinite(rate.get()) || rate.get() <= 0 || rate > Mbps(63.5))
            throw cRuntimeError("%s contains a legacy rate outside the wire range", name);
        if (std::fmod(rate.get(), 500000) != 0)
            throw cRuntimeError("%s contains a legacy rate that is not representable in 500 kbps units", name);
    }
    for (auto mcs : rateSet.htMcs)
        if (mcs < 0 || mcs >= 77)
            throw cRuntimeError("%s contains HT MCS index %d outside the range 0..76", name, mcs);
}

inline void validateIeee80211RateSetState(const Ieee80211RateSetState& state, const char *name)
{
    validateIeee80211RateSet(state.supported, "supported rate set");
    validateIeee80211RateSet(state.basic, "basic rate set");
    validateIeee80211RateSet(state.operational, "operational rate set");
    if (state.supported.known) {
        for (const auto *subset : {&state.basic, &state.operational}) {
            for (auto rate : subset->legacyRates)
                if (state.supported.legacyRates.count(rate) == 0)
                    throw cRuntimeError("%s contains an unsupported legacy rate", name);
            for (auto mcs : subset->htMcs)
                if (state.supported.htMcs.count(mcs) == 0)
                    throw cRuntimeError("%s contains an unsupported HT MCS", name);
        }
    }
    if (state.basic.known && state.operational.known) {
        for (auto rate : state.basic.legacyRates)
            if (state.operational.legacyRates.count(rate) == 0)
                throw cRuntimeError("%s basic legacy rate is absent from the operational rate set", name);
        for (auto mcs : state.basic.htMcs)
            if (state.operational.htMcs.count(mcs) == 0)
                throw cRuntimeError("%s basic HT MCS index is absent from the operational MCS set", name);
    }
}

} // namespace ieee80211
} // namespace inet

#endif
