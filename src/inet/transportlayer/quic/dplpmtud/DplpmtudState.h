//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATE_H_

#include "Dplpmtud.h"

namespace inet {
namespace quic {

class Dplpmtud;

class DplpmtudState {
public:
    DplpmtudState(Dplpmtud *context);
    virtual ~DplpmtudState();

    virtual DplpmtudState *onProbeAcked(int ackedProbeSize) = 0;
    virtual DplpmtudState *onProbeLost(int lostProbeSize) = 0;
    virtual DplpmtudState *onPtbReceived(int ptbMtu) = 0;
    virtual DplpmtudState *onPmtuInvalid(int largestAckedSinceLoss) = 0;
    virtual void onRaiseTimeout() = 0;

    virtual void onGotProbeSendPermission(int probeSizeLimit, int overhead) { }
    virtual bool canSendAnotherProbePacket(int probeSizeLimit, int overhead) { return false; }

protected:
    Dplpmtud *context;
    int probeCount;

    virtual void sendProbe(int probeSize, bool triggerSendRoutine = true);
    DplpmtudState *newState(DplpmtudState *state);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATE_H_ */
