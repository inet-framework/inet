//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCE_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCE_H_

namespace inet {
namespace quic {

class DplpmtudCandidateSequence {
public:
    DplpmtudCandidateSequence(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequence();

    virtual int getNextCandidate(int probeSizeLimit = (1<<16)) = 0;
    virtual void testSucceeded(int succeededCandidate) { }
    virtual void testFailed(int failedCandidate) { }
    virtual void setSmallestExpiredProbeSize(int size) {
        smallestExpiredProbeSize = size;
    }
    virtual bool repeatOnTimeout(int size) = 0;
    virtual void smallestProbeTimedOut(int size) { }
    virtual void ptbReceived(int ptbMtu) {
        maxPmtu = ptbMtu;
    }

protected:
    int minPmtu;
    int maxPmtu;
    int stepSize;
    int smallestExpiredProbeSize = (1<<16);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCE_H_ */
