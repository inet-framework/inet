//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudCandidateSequenceOptUp.h"

namespace inet {
namespace quic {

DplpmtudCandidateSequenceOptUp::DplpmtudCandidateSequenceOptUp(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequenceUp(minPmtu, maxPmtu, stepSize) {
    optProbe = true;
}
DplpmtudCandidateSequenceOptUp::~DplpmtudCandidateSequenceOptUp() { }

int DplpmtudCandidateSequenceOptUp::getNextCandidate(int probeSizeLimit) {
    static bool firstProbe = true;
    if (firstProbe) {
        firstProbe = false;
        return maxPmtu;
    } else if (optProbe) {
        optProbe = false;
    }
    int next = DplpmtudCandidateSequenceUp::getNextCandidate(probeSizeLimit);
    if (next == maxPmtu) {
        return 0;
    }
    return next;
}

bool DplpmtudCandidateSequenceOptUp::repeatOnTimeout(int size) {
    return !optProbe;
}


} /* namespace quic */
} /* namespace inet */
