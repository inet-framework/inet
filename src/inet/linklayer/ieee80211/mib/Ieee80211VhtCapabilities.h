// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211VHTCAPABILITIES_H
#define INET_IEEE80211VHTCAPABILITIES_H
#include "inet/linklayer/ieee80211/mib/Ieee80211HtCapabilities.h"

namespace inet::ieee80211 {

/** Model-backed uniform-NSS SU subset of 2024 9.4.2.157--158; no extended-NSS signaling. */
struct INET_API Ieee80211VhtCapabilities
{
    std::array<int, 8> rxMcs;
    std::array<int, 8> txMcs;
    bool width160 = false;
    bool shortGi80 = false;
    bool shortGi160 = false;
    int maxMpduLength = 3895;
    int maxAmpduLengthExponent = 0;
    int rxHighestRate = 0;
    int txHighestRate = 0;
    Ieee80211VhtCapabilities() { rxMcs.fill(-1); txMcs.fill(-1); }
    bool operator==(const Ieee80211VhtCapabilities& o) const {
        return rxMcs == o.rxMcs && txMcs == o.txMcs && width160 == o.width160 && shortGi80 == o.shortGi80
            && shortGi160 == o.shortGi160 && maxMpduLength == o.maxMpduLength && maxAmpduLengthExponent == o.maxAmpduLengthExponent
            && rxHighestRate == o.rxHighestRate && txHighestRate == o.txHighestRate;
    }
};

struct INET_API Ieee80211VhtOperation
{
    Hz width = MHz(20);
    uint8_t center0 = 0;
    uint8_t center1 = 0;
    uint16_t basicMcsMap = 0xFFFC;
    bool operator==(const Ieee80211VhtOperation& o) const {
        return width == o.width && center0 == o.center0 && center1 == o.center1 && basicMcsMap == o.basicMcsMap;
    }
};

struct INET_API Ieee80211PeerPhyCapabilities
{
    bool htPresent = false;
    bool vhtPresent = false;
    Ieee80211HtCapabilities ht;
    Ieee80211HtOperation htOperation;
    Ieee80211VhtCapabilities vht;
    Ieee80211VhtOperation vhtOperation;
    bool operator==(const Ieee80211PeerPhyCapabilities& o) const {
        return htPresent == o.htPresent && vhtPresent == o.vhtPresent && ht == o.ht && htOperation == o.htOperation
            && vht == o.vht && vhtOperation == o.vhtOperation;
    }
    bool operator!=(const Ieee80211PeerPhyCapabilities& o) const { return !(*this == o); }
};

inline uint16_t encodeVhtMcsMap(const std::array<int, 8>& map)
{
    uint16_t result = 0;
    for (int nss = 0; nss < 8; nss++) {
        int maximum = map[nss];
        if (maximum != -1 && maximum != 7 && maximum != 8 && maximum != 9)
            throw cRuntimeError("VHT MCS map entries must be -1,7,8,9");
        result |= (maximum == -1 ? 3 : maximum - 7) << (2 * nss);
    }
    return result;
}

inline std::array<int, 8> decodeVhtMcsMap(uint16_t value)
{
    std::array<int, 8> result;
    for (int nss = 0; nss < 8; nss++) {
        int code = (value >> (2 * nss)) & 3;
        result[nss] = code == 3 ? -1 : code + 7;
    }
    return result;
}

inline bool supportsBasicVhtMcs(const Ieee80211VhtCapabilities& c, uint16_t basic)
{
    auto map = decodeVhtMcsMap(basic);
    for (int nss = 0; nss < 8; nss++)
        if (map[nss] >= 0 && (c.rxMcs[nss] < map[nss] || c.txMcs[nss] < map[nss]))
            return false;
    return true;
}

} // namespace inet::ieee80211
#endif
