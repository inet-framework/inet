//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.h"

#include <algorithm>

#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/ratecontrol/RateStatistic.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t RateControlBase::datarateChangedSignal = cComponent::registerSignal("datarateChanged");

void RateControlBase::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        recordPerStationRate = par("recordPerStationRate");
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

MacAddress RateControlBase::getReceiverAddress(Packet *frame) const
{
    const auto& header = frame->peekAtFront<Ieee80211MacHeader>();
    return header->getReceiverAddress();
}

const IIeee80211Mode *RateControlBase::getInitialMode()
{
    double initialRate = par("initialRate");
    return initialRate == -1 ? modeSet->getFastestMandatoryMode() : modeSet->getMode(bps(initialRate));
}

void RateControlBase::emitDatarateChangedSignal(const MacAddress& receiver, const IIeee80211Mode *mode)
{
    bps rate = mode->getDataMode()->getNetBitrate();
    // aggregate statistic on this module (backward compatible: all stations interleaved)
    emit(datarateChangedSignal, rate.get());
    // per-station statistic on a dedicated submodule, so each station's rate is separately plottable
    if (recordPerStationRate && !receiver.isBroadcast() && !receiver.isMulticast())
        getOrCreateStationModule(receiver)->setRate(rate.get());
}

RateStatistic *RateControlBase::getOrCreateStationModule(const MacAddress& receiver)
{
    auto it = stationModules.find(receiver);
    if (it != stationModules.end())
        return it->second;
    auto moduleType = cModuleType::get("inet.linklayer.ieee80211.mac.ratecontrol.RateStatistic");
    // build a valid, MAC-derived submodule name, e.g. "sta-0AAA00000002"
    std::string macString = receiver.str();
    macString.erase(std::remove(macString.begin(), macString.end(), '-'), macString.end());
    std::string name = "sta-" + macString;
    auto module = check_and_cast<RateStatistic *>(moduleType->createScheduleInit(name.c_str(), this));
    module->setPeerAddress(receiver);
    stationModules[receiver] = module;
    return module;
}

void RateControlBase::clearStationModules()
{
    for (auto& it : stationModules)
        it.second->deleteModule();
    stationModules.clear();
}

void RateControlBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet *>(obj);
        resetRateControl();
        clearStationModules();
    }
}

} /* namespace ieee80211 */
} /* namespace inet */

