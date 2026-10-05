//
// Copyright (C) 2026 OpenSim Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later
//
#ifndef __INET_IEEE80211MGMTRATESET_H
#define __INET_IEEE80211MGMTRATESET_H

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211HtCapabilities.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211RateSet.h"

namespace inet {
namespace ieee80211 {

INET_API bool decodeMgmtRateSet(const Ieee80211SupportedRatesElement& rates, bool extendedPresent,
        const Ieee80211ExtendedSupportedRatesElement& extended, Ieee80211RateSetState& result);
INET_API void addHtRateSet(Ieee80211RateSetState& rates, const Ieee80211HtCapabilities& capabilities,
        const Ieee80211HtOperation *operation);
INET_API bool supportsBasicRateSet(const Ieee80211RateSetState& peer, const Ieee80211RateSetState& bss);

} // namespace ieee80211
} // namespace inet
#endif
