// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSRECORDSTATE_H
#define INET_DTLSRECORDSTATE_H

#include <cstdint>
#include <set>

#include "inet/common/INETDefs.h"

namespace inet::dtls {

/** Replay and sequence reconstruction state for one receive epoch. */
class INET_API DtlsRecordState
{
  protected:
    size_t windowSize;
    bool initialized = false;
    uint64_t highest = 0;
    std::set<uint64_t> received;

  public:
    explicit DtlsRecordState(size_t windowSize = 64);

    // RFC 9147 section 4.2.2: nearest candidate to highest + 1.
    [[nodiscard]] uint64_t reconstruct(uint16_t truncated, unsigned int numBits) const;
    [[nodiscard]] bool isAcceptable(uint64_t sequenceNumber) const;
    // The caller invokes this only after all modeled deprotection checks pass.
    bool accept(uint64_t sequenceNumber);
    // Distinguishes an untouched window from one that accepted record zero.
    [[nodiscard]] bool hasRecords() const { return initialized; }
    [[nodiscard]] uint64_t getHighestAccepted() const { ASSERT(initialized); return highest; }
};

} // namespace inet::dtls
#endif
