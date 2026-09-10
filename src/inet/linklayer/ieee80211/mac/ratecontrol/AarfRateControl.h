//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AARFRATECONTROL_H
#define __INET_AARFRATECONTROL_H

#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements the ARF and AARF rate control algorithms.
 */
class INET_API AarfRateControl : public RateControlBase
{
  protected:
    // Per-receiver state from RR-5208, Appendix A.
    struct State {
        MacAddress address; // the receiver this state belongs to (for per-station rate attribution)
        const physicallayer::IIeee80211Mode *mode = nullptr;
        int timer = 0;
        int timerThreshold = -1;
        bool probing = false; // recovery persists through failures until a successful transmission
        int increaseThreshold = -1;
        int numberOfConsSuccTransmissions = 0;
    };
    std::map<MacAddress, State> stations;

    // configuration, shared across stations
    int initialIncreaseThreshold = -1;
    int minTimerThreshold = -1;
    double timerThresholdFactor = -1;
    int maxIncreaseThreshold = -1;
    int decreaseThreshold = -1;
    double factor = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual State& getState(const MacAddress& receiverAddress);
    virtual void resetRateControl() override { stations.clear(); }

  public:
    virtual const physicallayer::IIeee80211Mode *getRate(const MacAddress& receiverAddress) override;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) override;
    virtual void frameReceived(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
