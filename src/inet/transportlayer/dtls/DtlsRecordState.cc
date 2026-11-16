// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsRecordState.h"

#include <limits>

namespace inet::dtls {

DtlsRecordState::DtlsRecordState(size_t windowSize) : windowSize(windowSize)
{
    if (windowSize == 0 || windowSize > 65536)
        throw cRuntimeError("DTLS replay window size must be between 1 and 65536");
}

uint64_t DtlsRecordState::reconstruct(uint16_t truncated, unsigned int numBits) const
{
    // RFC 9147 Section 4.2.2 reconstructs a truncated number around the expected next value.
    if (numBits != 8 && numBits != 16)
        throw cRuntimeError("DTLS record number width must be 8 or 16 bits");
    const uint64_t window = uint64_t(1) << numBits;
    const uint64_t mask = window - 1;
    if (uint64_t(truncated) > mask)
        throw cRuntimeError("DTLS truncated number exceeds its field width");
    const uint64_t expected = initialized && highest != std::numeric_limits<uint64_t>::max() ? highest + 1 : highest;
    uint64_t candidate = (expected & ~mask) | truncated;
    // Avoid overflow near UINT64_MAX. Ties retain the current high bits.
    if (candidate <= expected && expected - candidate > window / 2 && candidate <= std::numeric_limits<uint64_t>::max() - window)
        candidate += window;
    else if (candidate > expected && candidate - expected > window / 2 && candidate >= window)
        candidate -= window;
    return candidate;
}

bool DtlsRecordState::isAcceptable(uint64_t sequenceNumber) const
{
    // RFC 9147 Section 4.5.1 uses a sliding anti-replay window for each epoch.
    if (!initialized || sequenceNumber > highest)
        return true;
    if (highest - sequenceNumber >= windowSize)
        return false;
    return received.find(sequenceNumber) == received.end();
}

bool DtlsRecordState::accept(uint64_t sequenceNumber)
{
    if (!isAcceptable(sequenceNumber))
        return false;
    if (!initialized || sequenceNumber > highest) {
        highest = sequenceNumber;
        initialized = true;
        while (!received.empty() && highest - *received.begin() >= windowSize)
            received.erase(received.begin());
    }
    received.insert(sequenceNumber);
    return true;
}

} // namespace inet::dtls
