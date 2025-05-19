//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudState.h"

namespace inet {
namespace quic {

DplpmtudState::DplpmtudState(Dplpmtud *context) {
    this->context = context;
    probeCount = 0;
}

DplpmtudState::~DplpmtudState() { }

void DplpmtudState::sendProbe(int probeSize, bool triggerSendRoutine) {
    probeCount++;
    if (triggerSendRoutine) {
        context->sendProbe(probeSize);
    } else {
        context->prepareProbe(probeSize);
    }
}

DplpmtudState *DplpmtudState::newState(DplpmtudState *state) {
    delete this;
    return state;
}

} /* namespace quic */
} /* namespace inet */
