//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEJUMP_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEJUMP_H_

#include <vector>
#include <set>
#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceJump: public DplpmtudCandidateSequence {
public:
    DplpmtudCandidateSequenceJump(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceJump();

    virtual int getNextCandidate(int probeSizeLimit) override;
    virtual void testSucceeded(int succeededCandidate) override;
    virtual void testFailed(int failedCandidate) override;
    virtual bool repeatOnTimeout(int size) override;
    virtual void smallestProbeTimedOut(int size) override;

    /**
     * Removes all candidates larger than ptbMtu and add ptbMtu.
     *
     * @param ptbMtu The MTU reported by the PTB message.
     */
    virtual void ptbReceived(int ptbMtu) override;

private:
    std::vector<int> candidates;
    std::vector<int>::iterator currentIt;
    //std::set<int> usedCandidates;
    bool downward;

    /**
     * Add further candidates after the element the given iterator points to.
     *
     * @param after An iterator pointing to the element in candidates after that new candidates are to be added.
     */
    void addCandidates(std::vector<int>::iterator &after);

    /**
     * @param value Value in candidates to get an iterator for.
     * @return Iterator pointing to the element in candidates with the given value.
     */
    std::vector<int>::iterator getIterator(int value);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEJUMP_H_ */
