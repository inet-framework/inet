//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapObservation.h"

namespace inet {
namespace coap {
namespace observe {

bool isObserveOrderNewer(uint32_t currentVal, simtime_t currentTime,
        uint32_t candidateVal, simtime_t candidateTime)
{
    uint32_t v1 = currentVal & 0xFFFFFFU;
    uint32_t v2 = candidateVal & 0xFFFFFFU;
    constexpr uint32_t kHalfMod = (1U << 23U);

    if (candidateTime > currentTime + SimTime(128, SIMTIME_S))
        return true;
    if (v1 < v2 && (v2 - v1) < kHalfMod)
        return true;
    if (v1 > v2 && (v1 - v2) > kHalfMod)
        return true;
    return false;
}

uint64_t currentObserveTick(simtime_t now)
{
    if (now <= SIMTIME_ZERO)
        return 0;
    return static_cast<uint64_t>(now.inUnit(SIMTIME_MS));
}

} // namespace observe
} // namespace coap
} // namespace inet
