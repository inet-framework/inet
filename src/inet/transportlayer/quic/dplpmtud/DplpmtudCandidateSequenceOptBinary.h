//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEOPTBINARY_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEOPTBINARY_H_

#include "DplpmtudCandidateSequenceBinary.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceOptBinary: public DplpmtudCandidateSequenceBinary {
public:
    DplpmtudCandidateSequenceOptBinary(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceOptBinary();

    int getNextCandidate(int probeSizeLimit) override;
private:
    bool firstCandidate = true;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEOPTBINARY_H_ */
