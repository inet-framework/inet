// Copyright (C) 2026 INET Framework contributors
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211VHTCAPABILITIES_H
#define __INET_IEEE80211VHTCAPABILITIES_H

#include <array>
#include "inet/common/Units.h"

namespace inet {
namespace ieee80211 {

// IEEE Std 802.11-2024, 9.4.2.156.3: each map entry describes an inclusive
// MCS maximum (7, 8, 9), or -1 for an unsupported spatial-stream count.
struct INET_API Ieee80211VhtCapabilities
{
    std::array<int, 8> rxMaxMcs = {{-1, -1, -1, -1, -1, -1, -1, -1}};
    std::array<int, 8> txMaxMcs = {{-1, -1, -1, -1, -1, -1, -1, -1}};
    bool supported160Mhz = false;
    bool shortGi20 = false;
    bool shortGi40 = false;
    bool shortGi80 = false;
    bool shortGi160 = false;
    int rxHighestLongGiRateMbps = 0;
    int txHighestLongGiRateMbps = 0;
    bool operator==(const Ieee80211VhtCapabilities& other) const {
        return rxMaxMcs == other.rxMaxMcs && txMaxMcs == other.txMaxMcs && supported160Mhz == other.supported160Mhz &&
                shortGi20 == other.shortGi20 && shortGi40 == other.shortGi40 && shortGi80 == other.shortGi80 &&
                shortGi160 == other.shortGi160 && rxHighestLongGiRateMbps == other.rxHighestLongGiRateMbps &&
                txHighestLongGiRateMbps == other.txHighestLongGiRateMbps;
    }
};

struct INET_API Ieee80211VhtOperation
{
    Hz channelWidth = MHz(20);
    int centerFrequencySegment0 = 0;
    int centerFrequencySegment1 = 0;
    std::array<int, 8> basicMaxMcs = {{7, -1, -1, -1, -1, -1, -1, -1}};
    bool operator==(const Ieee80211VhtOperation& other) const {
        return channelWidth == other.channelWidth && centerFrequencySegment0 == other.centerFrequencySegment0 &&
                centerFrequencySegment1 == other.centerFrequencySegment1 && basicMaxMcs == other.basicMaxMcs;
    }
};

inline bool isValidVhtMcsMap(const std::array<int, 8>& map)
{
    for (int maximum : map)
        if (maximum != -1 && maximum != 7 && maximum != 8 && maximum != 9)
            return false;
    return map[0] >= 7;
}

inline bool supportsBasicVhtMcsSet(const Ieee80211VhtCapabilities& capabilities, const Ieee80211VhtOperation& operation)
{
    for (size_t nss = 0; nss < operation.basicMaxMcs.size(); nss++)
        if (operation.basicMaxMcs[nss] >= 0 &&
                (capabilities.rxMaxMcs[nss] < operation.basicMaxMcs[nss] || capabilities.txMaxMcs[nss] < operation.basicMaxMcs[nss]))
            return false;
    return true;
}

} // namespace ieee80211
} // namespace inet
#endif
