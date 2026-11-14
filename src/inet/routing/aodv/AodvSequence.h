// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_AODVSEQUENCE_H
#define __INET_AODVSEQUENCE_H

#include <cstdint>

namespace inet {
namespace aodv {

class AodvSequence
{
  public:
    static bool isNewer(uint32_t candidate, uint32_t stored)
    {
        // RFC 3561 section 6.1. Unsigned subtraction also handles wraparound.
        // Model decision: neither direction is newer at exactly half the range.
        uint32_t distance = candidate - stored;
        return distance != 0 && distance < UINT32_C(0x80000000);
    }

    static bool isAtLeast(uint32_t candidate, uint32_t stored)
    {
        return candidate == stored || isNewer(candidate, stored);
    }
};

} // namespace aodv
} // namespace inet

#endif
