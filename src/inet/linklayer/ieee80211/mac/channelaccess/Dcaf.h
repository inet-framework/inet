//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DCAF_H
#define __INET_DCAF_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"
#include "inet/linklayer/ieee80211/mac/contract/IContention.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"

namespace inet {
namespace ieee80211 {

class INET_API Dcaf : public IChannelAccess, public IContention::ICallback, public IRecoveryProcedure::ICwCalculator, public ModeSetListener
{
  protected:
    physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    IContention *contention = nullptr;
    IChannelAccess::ICallback *callback = nullptr;

    queueing::IPacketQueue *pendingQueue = nullptr;
    InProgressFrames *inProgressFrames = nullptr;

    bool owning = false;

    simtime_t slotTime = -1;
    simtime_t sifs = -1;
    simtime_t ifs = -1;
    simtime_t eifs = -1;

    int cw = -1;
    int cwMin = -1;
    int cwMax = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void calculateTimingParameters();
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  public:
    virtual queueing::IPacketQueue *getPendingQueue() const { return pendingQueue; }
    virtual InProgressFrames *getInProgressFrames() const { return inProgressFrames; }

    // IChannelAccess::ICallback
    virtual void requestChannel(IChannelAccess::ICallback *callback) override;
    virtual void releaseChannel(IChannelAccess::ICallback *callback) override;

    // IContention::ICallback
    virtual void channelAccessGranted() override;
    virtual void expectedChannelAccess(simtime_t time) override;

    // IRecoveryProcedure::ICallback
    virtual void incrementCw() override;
    virtual void resetCw() override;
    virtual int getCw() override { return cw; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

