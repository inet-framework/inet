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

    // The receiver of a frame the rate control is being given feedback about.
    virtual MacAddress getReceiverAddress(Packet *frame) const;
    // Emits datarateChanged with the receiver as a named details object, so a demux(datarateChanged)
    // result filter can record a separate data-rate vector per station. The aggregate datarateChanged
    // statistic ignores the details and is therefore unchanged. Group-addressed receivers are emitted
    // without details (aggregate only).
    virtual void emitDatarateChangedSignal(const MacAddress& receiver, const physicallayer::IIeee80211Mode *mode);

    const physicallayer::IIeee80211Mode *increaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
    const physicallayer::IIeee80211Mode *decreaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

