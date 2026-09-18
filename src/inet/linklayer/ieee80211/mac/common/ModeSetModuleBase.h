//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef INET_MODESETMODULEBASE_H
#define INET_MODESETMODULEBASE_H

#include "inet/common/SimpleModule.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee80211/mac/contract/IIeee80211MacConfiguration.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
namespace inet::ieee80211 {
/** Base for modules with a declared, configuration-lifetime catalog dependency. */
class INET_API ModeSetModuleBase : public SimpleModule
{
  protected:
    ModuleRefByPar<IIeee80211MacConfiguration> modeSetProvider;
    const physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
};
} // namespace inet::ieee80211
#endif
