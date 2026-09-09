//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"

#include <tuple>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {
namespace ieee80211 {

void ModeSetListener::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        getContainingNicModule(this)->subscribe(modesetChangedSignal, this);
}

void ModeSetListener::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    if (signalID == modesetChangedSignal && obj != modeSet)
        applyModeSet(check_and_cast<physicallayer::Ieee80211ModeSet *>(obj));
}

void ModeSetListener::applyModeSet(const physicallayer::Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    modeSet = const_cast<physicallayer::Ieee80211ModeSet *>(newModeSet);
}

std::function<void()> ModeSetListener::saveModeSetState()
{
    Enter_Method_Silent();
    return [this, state = std::make_tuple(modeSet)]() mutable {
        std::tie(modeSet) = std::move(state);
    };
}


} /* namespace ieee80211 */
} /* namespace inet */

