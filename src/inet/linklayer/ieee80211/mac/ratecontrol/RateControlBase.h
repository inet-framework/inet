//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RATECONTROLBASE_H
#define __INET_RATECONTROLBASE_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"

namespace inet {
namespace ieee80211 {

class INET_API RateControlBase : public ModeSetListener, public IRateControl
{
  public:
    static simsignal_t datarateChangedSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // The receiver MAC address of a transmitted (or received) frame, which keys the per-station state.
    virtual MacAddress getReceiverAddress(Packet *frame) const;
    // The mode a newly seen station starts from: the initialRate parameter, or the fastest mandatory mode.
    virtual const physicallayer::IIeee80211Mode *getInitialMode();
    virtual void emitDatarateChangedSignal(const physicallayer::IIeee80211Mode *mode);
    // Drops all per-station state; called by subclasses' override when the mode set changes.
    virtual void resetRateControl() {}

    const physicallayer::IIeee80211Mode *increaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
    const physicallayer::IIeee80211Mode *decreaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

