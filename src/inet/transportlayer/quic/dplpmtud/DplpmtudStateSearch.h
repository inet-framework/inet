//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATESEARCH_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATESEARCH_H_

#include "DplpmtudState.h"
#include "DplpmtudProbes.h"
#include "../Timer.h"
#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudStateSearch: public DplpmtudState {
public:
    DplpmtudStateSearch(Dplpmtud *context);
    virtual ~DplpmtudStateSearch();

    virtual DplpmtudState *onProbeAcked(int ackedProbeSize) override;
    virtual DplpmtudState *onProbeLost(int lostProbeSize) override;
    virtual DplpmtudState *onPtbReceived(int ptbMtu) override;
    virtual DplpmtudState *onPmtuInvalid(int largestAckedSinceLoss) override;
    virtual void onRaiseTimeout() override;

    virtual void onGotProbeSendPermission(int probeSizeLimit, int overhead) override;
    virtual bool canSendAnotherProbePacket(int probeSizeLimit, int overhead) override;

private:
    DplpmtudCandidateSequence *sequence;
    Timer *probeTimer;
    DplpmtudProbes outstandingProbes;
    int smallestExpiredProbeSize;
    std::set<int> testedCandidates;
    SimTime startSearchTime;
    SimTime endSearchTime;
    int networkLoad;

    void start();
    void sendProbe(int probeSize, bool triggerSendRoutine = true) override;
    void updateSmallestExpiredProbeSize();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATESEARCH_H_ */
