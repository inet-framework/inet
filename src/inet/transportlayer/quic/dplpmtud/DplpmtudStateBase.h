//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATEBASE_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATEBASE_H_

#include "DplpmtudState.h"

namespace inet {
namespace quic {

class DplpmtudStateBase: public DplpmtudState {
public:
    DplpmtudStateBase(Dplpmtud *context);
    virtual ~DplpmtudStateBase();

    virtual DplpmtudState *onProbeAcked(int ackedProbeSize) override;
    virtual DplpmtudState *onProbeLost(int lostProbeSize) override;
    virtual DplpmtudState *onPtbReceived(int ptbMtu) override;
    virtual DplpmtudState *onPmtuInvalid(int largestAckedSinceLoss) override;
    virtual void onRaiseTimeout() override;

private:
    int base;

    void start();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATEBASE_H_ */
