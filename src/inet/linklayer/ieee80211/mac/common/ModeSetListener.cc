//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"

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

    if (signalID == modesetChangedSignal)
        modeSet = check_and_cast<physicallayer::Ieee80211ModeSet *>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */

