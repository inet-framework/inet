//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RATECONTROLBASE_H
#define __INET_RATECONTROLBASE_H

#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"

namespace inet {
namespace ieee80211 {

class RateStatistic;

class INET_API RateControlBase : public ModeSetListener, public IRateControl
{
  public:
    static simsignal_t datarateChangedSignal;

  protected:
    // When true, a per-station RateStatistic submodule is created for each receiver so that
    // the data rate to each station is recorded as a separate, plottable statistic.
    bool recordPerStationRate = false;
    std::map<MacAddress, RateStatistic *> stationModules;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // The receiver MAC address of a transmitted (or received) frame, which keys the per-station state.
    virtual MacAddress getReceiverAddress(Packet *frame) const;
    // The mode a newly seen station starts from: the initialRate parameter, or the fastest mandatory mode.
    virtual const physicallayer::IIeee80211Mode *getInitialMode();
    // Emits the aggregate datarateChanged signal on this module (backward compatible), and, when
    // recordPerStationRate is enabled, also feeds the per-receiver RateStatistic submodule.
    virtual void emitDatarateChangedSignal(const MacAddress& receiver, const physicallayer::IIeee80211Mode *mode);
    // Finds or lazily creates the per-station RateStatistic submodule for the given receiver.
    virtual RateStatistic *getOrCreateStationModule(const MacAddress& receiver);
    // Deletes all per-station RateStatistic submodules (e.g. on a mode set change).
    virtual void clearStationModules();
    // Drops all per-station state; called by subclasses' override when the mode set changes.
    virtual void resetRateControl() {}

    const physicallayer::IIeee80211Mode *increaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
    const physicallayer::IIeee80211Mode *decreaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

