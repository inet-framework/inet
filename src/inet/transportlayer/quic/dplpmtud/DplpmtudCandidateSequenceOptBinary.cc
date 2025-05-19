//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudCandidateSequenceOptBinary.h"

namespace inet {
namespace quic {

DplpmtudCandidateSequenceOptBinary::DplpmtudCandidateSequenceOptBinary(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequenceBinary(minPmtu, maxPmtu, stepSize) { }
DplpmtudCandidateSequenceOptBinary::~DplpmtudCandidateSequenceOptBinary() { }

int DplpmtudCandidateSequenceOptBinary::getNextCandidate(int probeSizeLimit) {
    if (firstCandidate) {
        firstCandidate = false;
        return maxPmtu;
    }
    int next = DplpmtudCandidateSequenceBinary::getNextCandidate(probeSizeLimit);
    if (next == maxPmtu) {
        return 0;
    }
    return next;
}


} /* namespace quic */
} /* namespace inet */
