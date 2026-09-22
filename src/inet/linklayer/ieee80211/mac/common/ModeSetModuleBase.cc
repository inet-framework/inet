//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/common/ModeSetModuleBase.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetCoordinator.h"
namespace inet::ieee80211 {
void ModeSetModuleBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        modeSetProvider.reference(this, "modeSetModule", true);
        if (auto coordinator = dynamic_cast<physicallayer::IIeee80211ModeSetCoordinator *>(modeSetProvider.get()))
            coordinator->registerModeSetConsumer(this, physicallayer::IIeee80211ModeSetCoordinator::DERIVED_STATE);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        modeSet = modeSetProvider->getConfiguredModeSet();
        if (modeSet == nullptr)
            throw cRuntimeError("Configured IEEE 802.11 mode catalog is unavailable");
    }
}
void ModeSetModuleBase::applyModeSet(const physicallayer::Ieee80211ModeSet *newModeSet)
{
    Enter_Method_Silent();
    modeSet = newModeSet;
}
} // namespace inet::ieee80211
