//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211DSSSPLCP_H
#define __INET_IEEE80211DSSSPLCP_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

// IEEE Std 802.11-2024, 15.3.3 and 16.3.3: SIGNAL is the rate in
// 100 kbit/s units; LENGTH is PSDU airtime in whole microseconds.
inline uint16_t computeIeee80211DsssPlcpLength(int64_t octets, uint8_t signal)
{
    if (octets < 0 || (signal != 10 && signal != 20 && signal != 55 && signal != 110))
        throw cRuntimeError("Invalid DSSS PSDU length or SIGNAL");
    if (octets > 65535)
        throw cRuntimeError("DSSS PSDU exceeds PLCP LENGTH capacity");
    auto length = (octets * 80 + signal - 1) / signal;
    if (length > 65535)
        throw cRuntimeError("DSSS airtime exceeds PLCP LENGTH capacity");
    return length;
}

inline bool computeIeee80211DsssLengthExtension(int64_t octets, uint8_t signal, uint16_t length)
{
    return signal == 110 && int64_t(length) * 11 - octets * 8 >= 8;
}

// Returns -1 for a SIGNAL not defined by this PHY family. The raw wire fields
// remain available even when the decoded PSDU length is invalid.
inline int decodeIeee80211DsssPsduLength(uint8_t signal, uint8_t service, uint16_t length, bool highRate)
{
    if (signal != 10 && signal != 20 && !(highRate && (signal == 55 || signal == 110)))
        return -1;
    int octets = int(length) * signal / 80;
    if (signal == 110 && (service & 0x80))
        octets--;
    return octets;
}

} // namespace physicallayer
} // namespace inet

#endif
