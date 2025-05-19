//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEBINARY_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEBINARY_H_

#include <vector>
#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceBinary: public DplpmtudCandidateSequence {
public:
    DplpmtudCandidateSequenceBinary(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceBinary();

    virtual int getNextCandidate(int probeSizeLimit) override;
    virtual void testSucceeded(int succeededCandidate) override;
    virtual void testFailed(int failedCandidate) override;
    virtual bool repeatOnTimeout(int size) override;
    virtual void ptbReceived(int ptbMtu) override;

private:
    bool gotAck;
    std::vector<int> candidatesTree;
    std::vector<int> retainedCandidates;

    int calculateNextValue();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEBINARY_H_ */
