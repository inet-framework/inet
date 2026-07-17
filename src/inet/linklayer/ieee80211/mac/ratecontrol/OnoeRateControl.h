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
    // Per-receiver adaptive state (formerly single-instance module members).
    struct State {
        MacAddress address; // the receiver this state belongs to (for per-station rate attribution)
        const physicallayer::IIeee80211Mode *mode = nullptr;
        simtime_t timer = SIMTIME_ZERO;
        int numOfRetries = 0;
        int numOfSuccTransmissions = 0;
        int numOfGivenUpTransmissions = 0;
        double avgRetriesPerFrame = 0;
        int credit = 0;
    };
    std::map<MacAddress, State> stations;

    // configuration, shared across stations
    simtime_t interval = SIMTIME_ZERO;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual State& stateFor(const MacAddress& receiverAddress);
    virtual void resetRateControl() override { stations.clear(); }

    virtual void computeMode(State& state);
    virtual void resetStatisticalVariables(State& state);
    virtual void computeModeIfTimerIsExpired(State& state);
    virtual void refreshDisplay() const override;

  public:
    virtual const physicallayer::IIeee80211Mode *getRate(const MacAddress& receiverAddress) override;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) override;
    virtual void frameReceived(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

