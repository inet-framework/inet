//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/common/ModeSetModuleBase.h"
namespace inet::ieee80211 {
void ModeSetModuleBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        modeSetProvider.reference(this, "modeSetModule", true);
    else if (stage == INITSTAGE_LINK_LAYER) {
        modeSet = modeSetProvider->getConfiguredModeSet();
        if (modeSet == nullptr)
            throw cRuntimeError("Configured IEEE 802.11 mode catalog is unavailable");
    }
}
} // namespace inet::ieee80211
