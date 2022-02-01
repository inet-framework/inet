//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTENTION_H
#define __INET_CONTENTION_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/contract/IContention.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace ieee80211 {

/**
 * The default implementation of IContention.
 */
class INET_API Contention : public cSimpleModule, public IContention
{
  public:
    enum State { IDLE, DEFER, IFS_AND_BACKOFF };
    enum EventType { START, MEDIUM_STATE_CHANGED, CORRUPTED_FRAME_RECEIVED, CHANNEL_ACCESS_GRANTED };
    static simsignal_t stateChangedSignal;

  protected:
    Ieee80211Mac *mac = nullptr;
    ICallback *callback = nullptr;
    cMessage *startTxEvent = nullptr;
    cMessage *channelGrantedEvent = nullptr;

    // current contention's parameters
    simtime_t ifs = SIMTIME_ZERO;
    simtime_t eifs = SIMTIME_ZERO;
    simtime_t slotTime = SIMTIME_ZERO;

    cFSM fsm;
    simtime_t endEifsTime = SIMTIME_ZERO;
    int backoffSlots = 0;
    simtime_t scheduledTransmissionTime = SIMTIME_ZERO;
    simtime_t lastChannelBusyTime = SIMTIME_ZERO;
    simtime_t lastIdleStartTime = SIMTIME_ZERO;
    simtime_t backoffOptimizationDelta = SIMTIME_ZERO;
    bool mediumFree = true;
    bool backoffOptimization = true;
    simtime_t startTime = SIMTIME_ZERO; // TODO for debugging

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void handleWithFSM(EventType event);
    virtual void scheduleTransmissionRequest();
    virtual void scheduleTransmissionRequestFor(simtime_t txStartTime);
    virtual void cancelTransmissionRequest();
    virtual void switchToEifs();
    virtual void computeRemainingBackoffSlots();
    virtual void revokeBackoffOptimization();
    virtual void updateDisplayString(simtime_t expectedChannelAccess) const;
    const char *getEventName(EventType event);

  public:
    Contention() {}
    ~Contention();

    // TODO also add a switchToReception() method? because switching takes time, so we dont automatically switch to tx after completing a transmission! (as we may want to transmit immediate frames afterwards)
    virtual void startContention(int cw, simtime_t ifs, simtime_t eifs, simtime_t slotTime, ICallback *callback) override;

    virtual void mediumStateChanged(bool mediumFree) override;
    virtual void corruptedFrameReceived() override;
    virtual bool isContentionInProgress() override { return fsm.getState() != IDLE; }
};

} // namespace ieee80211
} // namespace inet

#endif

