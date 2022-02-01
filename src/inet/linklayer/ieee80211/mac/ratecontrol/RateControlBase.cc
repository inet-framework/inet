//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t RateControlBase::datarateChangedSignal = cComponent::registerSignal("datarateChanged");

void RateControlBase::initialize(int stage)
{
    ModeSetListener::initialize(stage);
}

const IIeee80211Mode *RateControlBase::increaseRateIfPossible(const IIeee80211Mode *currentMode)
{
    const IIeee80211Mode *newMode = modeSet->getFasterMode(currentMode);
    return newMode == nullptr ? currentMode : newMode;
}

const IIeee80211Mode *RateControlBase::decreaseRateIfPossible(const IIeee80211Mode *currentMode)
{
    const IIeee80211Mode *newMode = modeSet->getSlowerMode(currentMode);
    return newMode == nullptr ? currentMode : newMode;
}

void RateControlBase::emitDatarateChangedSignal()
{
    bps rate = currentMode->getDataMode()->getNetBitrate();
    emit(datarateChangedSignal, rate.get());
}

void RateControlBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet *>(obj);
        double initRate = par("initialRate");
        currentMode = initRate == -1 ? modeSet->getFastestMandatoryMode() : modeSet->getMode(bps(initRate));
        emitDatarateChangedSignal();
    }
}

} /* namespace ieee80211 */
} /* namespace inet */

