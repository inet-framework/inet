//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEDOWN_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEDOWN_H_

#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceDown: public DplpmtudCandidateSequence {
public:
    DplpmtudCandidateSequenceDown(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceDown();

    virtual int getNextCandidate(int probeSizeLimit) override;
    virtual void testSucceeded(int succeededCandidate) override;
    virtual void testFailed(int failedCandidate) override;
    virtual bool repeatOnTimeout(int size) override;

private:
    int currentCandidate;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEDOWN_H_ */
