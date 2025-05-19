//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DplpmtudStateBase.h"
#include "DplpmtudStateSearch.h"
#include "DplpmtudStateComplete.h"

namespace inet {
namespace quic {

DplpmtudStateBase::DplpmtudStateBase(Dplpmtud *context) : DplpmtudState(context) {
    start();
}
DplpmtudStateBase::~DplpmtudStateBase() { }

void DplpmtudStateBase::start() {
    context->resetMinPmtu();
    base = context->getMinPmtu();
    context->setPmtu(base);
    probeCount = 0;
    EV_DEBUG << "DPLPMTUD in BASE: send probe for " << base << endl;
    sendProbe(base);
}

DplpmtudState *DplpmtudStateBase::onProbeAcked(int ackedProbeSize) {
    if (ackedProbeSize != base) {
        return this;
    }
    if (ackedProbeSize == context->getMaxPmtu()) {
        EV_DEBUG << "DPLPMTUD in BASE: max PMTU acked, transition to COMPLETE" << endl;
        return newState(new DplpmtudStateComplete(context));
    }
    EV_DEBUG << "DPLPMTUD in BASE: probe acked, transition to SEARCH " << endl;
    context->getPath()->getConnection()->onDplpmtudLeftBase();
    return newState(new DplpmtudStateSearch(context));
}

DplpmtudState *DplpmtudStateBase::onProbeLost(int lostProbeSize) {
    if (lostProbeSize != base) {
        return this;
    }
    if (probeCount < context->getMaxProbes()) {
        EV_DEBUG << "DPLPMTUD in BASE: probe lost, repeat. " << endl;
        sendProbe(base);
        return this;
    }

    throw cRuntimeError("DPLPMTUD: Failed to probe in base state");
}

DplpmtudState *DplpmtudStateBase::onPtbReceived(int ptbMtu) {
    EV_DEBUG << "DPLPMTUD in BASE: PTB received" << endl;
    // already in base
    return this;
}

DplpmtudState *DplpmtudStateBase::onPmtuInvalid(int largestAckedSinceLoss) {
    return this;
}

void DplpmtudStateBase::onRaiseTimeout() {
    throw cRuntimeError("DPLPMTUD: Raise Timeout in base state should not happen");
    //return this;
}

} /* namespace quic */
} /* namespace inet */
