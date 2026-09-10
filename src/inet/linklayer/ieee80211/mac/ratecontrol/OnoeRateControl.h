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
 * Implements Onoe's completed-sample rate adaptation rules.
 */
class INET_API OnoeRateControl : public RateControlBase
{
  protected:
    // Completed-sample statistics and adaptation state belong to each receiver.
    struct State {
        MacAddress address; // the receiver this state belongs to (for per-station rate attribution)
        const physicallayer::IIeee80211Mode *mode = nullptr;
        simtime_t timer = SIMTIME_ZERO;
        int64_t numOfRetries = 0; // failed attempts of completed frames, including terminal failures
        int64_t numOfSuccTransmissions = 0;
        int64_t numOfGivenUpTransmissions = 0;
        int credit = 0; // 0 through 9 after each evaluation
    };
    std::map<MacAddress, State> stations;

    // configuration, shared across stations
    simtime_t interval = SIMTIME_ZERO;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual State& getState(const MacAddress& receiverAddress);
    virtual void resetRateControl() override { stations.clear(); }

    virtual void computeMode(State& state);
    virtual void resetStatisticalVariables(State& state);
    virtual void computeModeIfTimerIsExpired(State& state);

  public:
    virtual const physicallayer::IIeee80211Mode *getRate(const MacAddress& receiverAddress) override;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) override;
    virtual void frameTransmitted(Packet *frame, int retryCount, int totalRetryCount, bool isSuccessful, bool isGivenUp) override;
    virtual void rtsFrameTransmissionFailed(Packet *frame, int totalRetryCount, bool isGivenUp) override;
    virtual void frameReceived(Packet *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
