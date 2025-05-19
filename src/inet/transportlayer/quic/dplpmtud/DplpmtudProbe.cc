//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudProbe.h"

namespace inet {
namespace quic {

DplpmtudProbe::DplpmtudProbe(int probeSize, SimTime timeSent, int probeCount) {
    this->probeSize = probeSize;
    this->timeSent = timeSent;
    this->probeCount = probeCount;
}

DplpmtudProbe::~DplpmtudProbe() { }

} /* namespace quic */
} /* namespace inet */
