//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_VECTORSEQUENCERECOVERY_H
#define __INET_VECTORSEQUENCERECOVERY_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Packet-independent state for IEEE Std 802.1CB-2017, 7.4.3.2 through 7.4.3.6,
 * with the IEEE 802.1CBdb-2021 forward-boundary correction (item 0257).
 * Each instance belongs to one sequence-number space. The caller supplies timers,
 * packet forwarding, tagless handling, and counters. Queries never change state.
 */
class INET_API VectorSequenceRecovery
{
  public:
    // IEEE Std 802.1CB-2017, 7.4.3.2.1: fixed 16-bit sequence-number space.
    static constexpr int SEQUENCE_NUMBER_SPACE = 65536;
    static constexpr int HALF_SEQUENCE_NUMBER_SPACE = SEQUENCE_NUMBER_SPACE / 2;
    // 10.4.1.6 defines the minimum/default; the maximum is implementation-defined.
    static constexpr int MIN_HISTORY_LENGTH = 2;
    static constexpr int DEFAULT_HISTORY_LENGTH = MIN_HISTORY_LENGTH;
    static constexpr int MAX_HISTORY_LENGTH = HALF_SEQUENCE_NUMBER_SPACE;

    enum Decision { ACCEPT, ACCEPT_OUT_OF_ORDER, DUPLICATE, ROGUE };

    struct Result {
        Decision decision;
        int lostPackets;
    };

  protected:
    std::vector<bool> history;
    uint16_t recoveredSequenceNumber = SEQUENCE_NUMBER_SPACE - 1;
    bool takeAny = true;

    int computeDelta(uint16_t sequenceNumber) const;
    int advanceHistory(int distance);

  public:
    explicit VectorSequenceRecovery(int historyLength = DEFAULT_HISTORY_LENGTH);
    void reset();
    Decision classify(uint16_t sequenceNumber) const;
    Result process(uint16_t sequenceNumber);
};

} // namespace inet

#endif
