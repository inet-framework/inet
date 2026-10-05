//
// Copyright (C) 2026 OpenSim Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtRateSet.h"

namespace inet {
namespace ieee80211 {

bool decodeMgmtRateSet(const Ieee80211SupportedRatesElement& rates, bool extendedPresent,
        const Ieee80211ExtendedSupportedRatesElement& extended, Ieee80211RateSetState& result)
{
    // IEEE Std 802.11-2024, 9.4.2.3 and 9.4.2.11: validate both elements
    // before the caller changes discovery or association state.
    if (rates.numRates < 1 || rates.numRates > 8 ||
            (extendedPresent && (extended.numRates < 1 || extended.numRates > 255)))
        return false;
    Ieee80211RateSetState candidate;
    candidate.supported.known = candidate.basic.known = candidate.operational.known = true;
    auto insert = [&](double rate, bool basic) {
        if (!std::isfinite(rate) || rate <= 0 || rate > 63.5 || std::fmod(rate, 0.5) != 0)
            return false;
        bps value = Mbps(rate);
        if (!candidate.supported.legacyRates.insert(value).second)
            return false;
        candidate.operational.legacyRates.insert(value);
        if (basic)
            candidate.basic.legacyRates.insert(value);
        return true;
    };
    for (int i = 0; i < rates.numRates; ++i)
        if (!insert(rates.rate[i], rates.basicRate[i]))
            return false;
    if (extendedPresent)
        for (int i = 0; i < extended.numRates; ++i)
            if (!insert(extended.rate[i], extended.basicRate[i]))
                return false;
    result = candidate;
    return true;
}

void addHtRateSet(Ieee80211RateSetState& rates, const Ieee80211HtCapabilities& capabilities,
        const Ieee80211HtOperation *operation)
{
    for (int mcs = 0; mcs < 77; ++mcs) {
        if (capabilities.rxMcsSupported[mcs]) {
            rates.supported.htMcs.insert(mcs);
            rates.operational.htMcs.insert(mcs);
        }
        if (operation != nullptr && operation->basicMcsSupported[mcs])
            rates.basic.htMcs.insert(mcs);
    }
}

bool supportsBasicRateSet(const Ieee80211RateSetState& peer, const Ieee80211RateSetState& bss)
{
    if (!peer.supported.known || !bss.basic.known)
        return false;
    for (auto rate : bss.basic.legacyRates)
        if (peer.supported.legacyRates.count(rate) == 0)
            return false;
    return true; // The existing HT negotiation checks Basic HT-MCS separately.
}

} // namespace ieee80211
} // namespace inet
