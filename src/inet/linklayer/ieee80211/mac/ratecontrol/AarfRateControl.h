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
    simtime_t timer = SIMTIME_ZERO;
    simtime_t interval = SIMTIME_ZERO;
    bool probing = false;
    int increaseThreshold = -1;
    int maxIncreaseThreshold = -1;
    int decreaseThreshold = -1;
    double factor = -1;

    int numberOfConsSuccTransmissions = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void multiplyIncreaseThreshold(double factor);
    virtual void resetIncreaseThreshdold();
    virtual void resetTimer();
    virtual void increaseRateIfTimerIsExpired();
    virtual void updateDisplayString() const;

  public:
    virtual const physicallayer::IIeee80211Mode *getRate() override;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) override;
    virtual void frameReceived(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

