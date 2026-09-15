//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211DURATION_H
#define __INET_IEEE80211DURATION_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

inline simtime_t normalizeIeee80211Duration(simtime_t duration)
{
    // IEEE Std 802.11-2024, 9.2.5.1: negative calculations become zero; fractions round upward.
    if (duration <= SIMTIME_ZERO)
        return SIMTIME_ZERO;
    // 9.2.4.2, Table 9-9: ordinary Duration is 15 bits. AID is a separate representation.
    if (duration > SimTime(32767, SIMTIME_US))
        throw cRuntimeError("Calculated IEEE 802.11 Duration exceeds 32767 microseconds");
    auto microseconds = duration.inUnit(SIMTIME_US);
    if (SimTime(microseconds, SIMTIME_US) < duration)
        microseconds++;
    return SimTime(microseconds, SIMTIME_US);
}

inline uint16_t encodeIeee80211Duration(simtime_t duration)
{
    if (duration < SIMTIME_ZERO)
        throw cRuntimeError("Cannot serialize an unset or negative IEEE 802.11 Duration");
    return static_cast<uint16_t>(normalizeIeee80211Duration(duration).inUnit(SIMTIME_US));
}

} // namespace ieee80211
} // namespace inet

#endif
