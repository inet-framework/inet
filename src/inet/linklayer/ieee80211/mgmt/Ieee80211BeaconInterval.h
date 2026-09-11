//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211BEACONINTERVAL_H
#define __INET_IEEE80211BEACONINTERVAL_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

inline simtime_t normalizeIeee80211BeaconInterval(simtime_t interval)
{
    // IEEE Std 802.11-2024, 9.4.1.3: a 16-bit count of 1024 us TUs.
    // INET policy: round down once, matching the historical wire encoding,
    // and use the resulting duration for both TBTTs and advertisements.
    if (interval < SimTime(1024, SIMTIME_US) || interval > SimTime(65535LL * 1024, SIMTIME_US))
        throw cRuntimeError("Beacon interval must be between 1 and 65535 TUs (1024 us each)");
    return SimTime(interval.inUnit(SIMTIME_US) / 1024 * 1024, SIMTIME_US);
}

} // namespace ieee80211
} // namespace inet

#endif
