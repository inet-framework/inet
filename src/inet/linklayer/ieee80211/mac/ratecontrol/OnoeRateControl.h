//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ONOERATECONTROL_H
#define __INET_ONOERATECONTROL_H

#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements the Onoe rate control algorithms.
 */
class INET_API OnoeRateControl : public RateControlBase
{
  protected:
    simtime_t timer = SIMTIME_ZERO;
    simtime_t interval = SIMTIME_ZERO;

    int numOfRetries = 0;
    int numOfSuccTransmissions = 0;
    int numOfGivenUpTransmissions = 0;

    double avgRetriesPerFrame = 0;
    int credit = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void computeMode();
    virtual void resetStatisticalVariables();
    virtual void computeModeIfTimerIsExpired();
    virtual void updateDisplayString() const;

  public:
    virtual const physicallayer::IIeee80211Mode *getRate() override;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) override;
    virtual void frameReceived(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

