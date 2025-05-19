//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudCandidateSequenceDown.h"

#include "Dplpmtud.h"

namespace inet {
namespace quic {

DplpmtudCandidateSequenceDown::DplpmtudCandidateSequenceDown(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequence(minPmtu, maxPmtu, stepSize) {
    currentCandidate = 0;
}
DplpmtudCandidateSequenceDown::~DplpmtudCandidateSequenceDown() { }

void DplpmtudCandidateSequenceDown::testSucceeded(int candidate) {
    minPmtu = std::max(minPmtu, candidate);
}

void DplpmtudCandidateSequenceDown::testFailed(int candidate) {
    maxPmtu = std::min(maxPmtu, candidate-stepSize);
}

int DplpmtudCandidateSequenceDown::getNextCandidate(int probeSizeLimit) {
    int next = currentCandidate - stepSize;
    if (currentCandidate == 0 || currentCandidate > maxPmtu) {
        currentCandidate = maxPmtu;
        next = currentCandidate;
    }
    EV_DEBUG << "DplpmtudSearchAlgorithmDown::getNextCandidate currentCandidate=" << currentCandidate << ", next=" << next << ", probeSizeLimit=" << probeSizeLimit << endl;
    if (next >= smallestExpiredProbeSize || !(minPmtu <= next && next <= maxPmtu) || next > probeSizeLimit) {
        return 0;
    }
    currentCandidate = next;
    return next;
}

bool DplpmtudCandidateSequenceDown::repeatOnTimeout(int size) {
    return (minPmtu >= currentCandidate || minPmtu >= size);
}

} /* namespace quic */
} /* namespace inet */
