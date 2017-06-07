//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/NotifierConsts.h"
#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.h"

namespace inet {
namespace ieee80211 {

simsignal_t RateControlBase::datarateSignal = cComponent::registerSignal("datarate");

void RateControlBase::initialize(int stage)
{
    ModeSetListener::initialize(stage);
}

const IIeee80211Mode* RateControlBase::increaseRateIfPossible(const IIeee80211Mode* currentMode)
{
    const IIeee80211Mode *newMode = modeSet->getFasterMode(currentMode);
    return newMode == nullptr ? currentMode : newMode;
}

const IIeee80211Mode* RateControlBase::decreaseRateIfPossible(const IIeee80211Mode* currentMode)
{
    const IIeee80211Mode *newMode = modeSet->getSlowerMode(currentMode);
    return newMode == nullptr ? currentMode : newMode;
}

void RateControlBase::emitDatarateSignal()
{
    bps rate = currentMode->getDataMode()->getNetBitrate();
    emit(datarateSignal, rate.get());
}

void RateControlBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED) {
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
        double initRate = par("initialRate");
        currentMode = initRate == -1 ? modeSet->getFastestMandatoryMode() : modeSet->getMode(bps(initRate));
        emitDatarateSignal();
    }
}

} /* namespace ieee80211 */
} /* namespace inet */
