//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RATECONTROLBASE_H
#define __INET_RATECONTROLBASE_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"

namespace inet {
namespace ieee80211 {

class INET_API RateControlBase : public ModeSetListener, public IRateControl
{
  public:
    static simsignal_t datarateChangedSignal;

  protected:
    const physicallayer::IIeee80211Mode *currentMode = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void emitDatarateChangedSignal();

    const physicallayer::IIeee80211Mode *increaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
    const physicallayer::IIeee80211Mode *decreaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

