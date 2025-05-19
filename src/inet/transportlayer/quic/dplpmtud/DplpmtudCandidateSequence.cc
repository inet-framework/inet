//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

DplpmtudCandidateSequence::DplpmtudCandidateSequence(int minPmtu, int maxPmtu, int stepSize) {
    this->minPmtu = minPmtu;
    this->maxPmtu = maxPmtu;
    this->stepSize = stepSize;
}

DplpmtudCandidateSequence::~DplpmtudCandidateSequence() { }

} /* namespace quic */
} /* namespace inet */
