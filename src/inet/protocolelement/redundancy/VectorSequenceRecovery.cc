//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/protocolelement/redundancy/VectorSequenceRecovery.h"

#include <stdexcept>
#include <string>

namespace inet {

VectorSequenceRecovery::VectorSequenceRecovery(int historyLength)
{
    if (historyLength < MIN_HISTORY_LENGTH || historyLength > MAX_HISTORY_LENGTH)
        throw std::invalid_argument("Recovery history length must be between " +
                                    std::to_string(MIN_HISTORY_LENGTH) + " and " + std::to_string(MAX_HISTORY_LENGTH));
    history.resize(historyLength);
    reset();
}

void VectorSequenceRecovery::reset()
{
    recoveredSequenceNumber = SEQUENCE_NUMBER_SPACE - 1;
    std::fill(history.begin(), history.end(), false);
    takeAny = true;
}

int VectorSequenceRecovery::computeDelta(uint16_t sequenceNumber) const
{
    // Signed modular difference, including the half-space case (7.4.3.4).
    int delta = (int(sequenceNumber) - int(recoveredSequenceNumber) + SEQUENCE_NUMBER_SPACE)
                % SEQUENCE_NUMBER_SPACE;
    return delta >= HALF_SEQUENCE_NUMBER_SPACE ? delta - SEQUENCE_NUMBER_SPACE : delta;
}

VectorSequenceRecovery::Decision VectorSequenceRecovery::classify(uint16_t sequenceNumber) const
{
    if (takeAny)
        return ACCEPT;
    int delta = computeDelta(sequenceNumber);
    int historyLength = history.size();
    // IEEE 802.1CBdb-2021 maintenance item 0257 includes the forward endpoint.
    if (delta > historyLength || delta <= -historyLength)
        return ROGUE;
    if (delta <= 0)
        return history[-delta] ? DUPLICATE : ACCEPT_OUT_OF_ORDER;
    return delta == 1 ? ACCEPT : ACCEPT_OUT_OF_ORDER;
}

int VectorSequenceRecovery::advanceHistory(int distance)
{
    // Equivalent to distance calls to 7.4.3.6, with the last call inserting 1.
    // distance <= history length, so only old bits leave the history. Count their
    // zeros before shifting once; this avoids quadratic work for large gaps.
    int lost = 0;
    int historyLength = history.size();
    for (int i = historyLength - distance; i < historyLength; i++)
        if (!history[i])
            lost++;
    for (int i = historyLength - 1; i >= distance; i--)
        history[i] = history[i - distance];
    std::fill(history.begin(), history.begin() + distance, false);
    history[0] = true;
    return lost;
}

VectorSequenceRecovery::Result VectorSequenceRecovery::process(uint16_t sequenceNumber)
{
    Decision decision = classify(sequenceNumber);
    int lost = 0;
    if (decision == DUPLICATE || decision == ROGUE)
        return {decision, lost};
    if (takeAny) {
        takeAny = false;
        history[0] = true;
        recoveredSequenceNumber = sequenceNumber;
    }
    else {
        int delta = computeDelta(sequenceNumber);
        if (delta <= 0)
            history[-delta] = true;
        else {
            lost = advanceHistory(delta);
            recoveredSequenceNumber = sequenceNumber;
        }
    }
    return {decision, lost};
}

} // namespace inet
