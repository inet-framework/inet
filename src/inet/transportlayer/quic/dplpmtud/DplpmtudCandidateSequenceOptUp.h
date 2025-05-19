//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEOPTUP_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEOPTUP_H_

#include "DplpmtudCandidateSequenceUp.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceOptUp: public DplpmtudCandidateSequenceUp {
public:
    DplpmtudCandidateSequenceOptUp(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceOptUp();

    virtual int getNextCandidate(int probeSizeLimit) override;
    virtual bool repeatOnTimeout(int size) override;

private:
    bool optProbe;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEOPTUP_H_ */
