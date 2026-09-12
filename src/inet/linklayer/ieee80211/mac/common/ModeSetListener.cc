//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetCoordinator.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {
namespace ieee80211 {

void ModeSetListener::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        check_and_cast<physicallayer::IIeee80211ModeSetCoordinator *>(getContainingNicModule(this))->registerModeSetConsumer(this, physicallayer::IIeee80211ModeSetCoordinator::DERIVED_STATE);
}

void ModeSetListener::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    // Mode-set application uses the coordinator contract, not notifications.
}

void ModeSetListener::applyModeSet(const physicallayer::Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    modeSet = const_cast<physicallayer::Ieee80211ModeSet *>(newModeSet);
}

} /* namespace ieee80211 */
} /* namespace inet */

