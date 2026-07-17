//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RATECONTROLBASE_H
#define __INET_RATECONTROLBASE_H

#include <string>

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
    // The demux label identifying a receiver in the per-station datarate statistic (the receiver MAC).
    virtual std::string stationLabel(const MacAddress& receiver) const;
    // Emits datarateChanged with the receiver as a named details object, so a demux(datarateChanged)
    // result filter can record a separate data-rate vector per station. The aggregate datarateChanged
    // statistic ignores the details and is therefore unchanged. Group-addressed receivers are emitted
    // without details (aggregate only).
    virtual void emitDatarateChangedSignal(const MacAddress& receiver, const physicallayer::IIeee80211Mode *mode);
    // Drops all per-station state; called by subclasses' override when the mode set changes.
    virtual void resetRateControl() {}

    const physicallayer::IIeee80211Mode *increaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
    const physicallayer::IIeee80211Mode *decreaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

