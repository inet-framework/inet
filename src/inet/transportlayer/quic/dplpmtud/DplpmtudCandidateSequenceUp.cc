//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudCandidateSequenceUp.h"

#include "Dplpmtud.h"

namespace inet {
namespace quic {

DplpmtudCandidateSequenceUp::DplpmtudCandidateSequenceUp(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequence(minPmtu, maxPmtu, stepSize) {
    currentCandidate = minPmtu;
}
DplpmtudCandidateSequenceUp::~DplpmtudCandidateSequenceUp() { }

void DplpmtudCandidateSequenceUp::testFailed(int candidate) {
    maxPmtu = std::min(maxPmtu, candidate);
}

int DplpmtudCandidateSequenceUp::getNextCandidate(int probeSizeLimit) {
    int next = currentCandidate + stepSize;
    EV_DEBUG << "DplpmtudSearchAlgorithmUp::getNextCandidate currentCandidate=" << currentCandidate << ", next=" << next << ", probeSizeLimit=" << probeSizeLimit << endl;
    if (next >= smallestExpiredProbeSize || next > maxPmtu || next > probeSizeLimit) {
        return 0;
    }
    currentCandidate = next;
    return next;
}

bool DplpmtudCandidateSequenceUp::repeatOnTimeout(int size) {
    return true;
}

} /* namespace quic */
} /* namespace inet */
