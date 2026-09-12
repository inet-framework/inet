//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/Ieee80211Interface.h"

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetListener.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(Ieee80211Interface);

void Ieee80211Interface::initialize(int stage)
{
    NetworkInterface::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        auto mac = check_and_cast<IIeee80211ModeSetListener *>(getSubmodule("mac"));
        modeSet = mac->getModeSet();
        beginModeSetChange(modeSet);
        completeModeSetChange(modeSet);
        modeSetInitialized = true;
    }
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        // All child link-layer wiring is ready before the initial completed fact.
        changingModeSet = true;
        modeSetPublished = true;
        emit(modesetChangedSignal, const_cast<Ieee80211ModeSet *>(modeSet));
        changingModeSet = false;
    }
}

void Ieee80211Interface::registerModeSetConsumer(cModule *consumer, Phase phase)
{
    Enter_Method_Silent();
    if (changingModeSet || modeSetInitialized)
        throw cRuntimeError("Mode-set consumers must register before initialization completes");
    if (consumer == nullptr || getContainingNicModule(consumer) != this ||
            dynamic_cast<IIeee80211ModeSetListener *>(consumer) == nullptr)
        throw cRuntimeError("Mode-set consumer must implement the contract in this interface");
    if (phase != MAC_STATE && phase != DERIVED_STATE)
        throw cRuntimeError("Invalid mode-set consumer phase");
    if ((consumer == getSubmodule("mac")) != (phase == MAC_STATE))
        throw cRuntimeError("Only the configured MAC owns the mode-set MAC_STATE phase");
    auto result = modeSetConsumers.emplace(consumer->getId(), phase);
    if (!result.second && result.first->second != phase)
        throw cRuntimeError("Mode-set consumer registered in two phases");
}

void Ieee80211Interface::unregisterModeSetConsumer(cModule *consumer)
{
    Enter_Method_Silent();
    if (changingModeSet)
        throw cRuntimeError("Cannot detach a mode-set consumer during a transition");
    if (consumer == nullptr || getContainingNicModule(consumer) != this)
        throw cRuntimeError("Cannot detach a mode-set consumer from another interface");
    modeSetConsumers.erase(consumer->getId());
}

void Ieee80211Interface::beginModeSetChange(const Ieee80211ModeSet *modeSet)
{
    Enter_Method_Silent();
    if (changingModeSet)
        throw cRuntimeError("Reentrant interface mode-set change");
    if (modeSet == nullptr)
        throw cRuntimeError("Cannot clear the mode set of an IEEE 802.11 interface");
    for (const char *name : {"mac", "mgmt"}) {
        auto module = getSubmodule(name);
        if (module == nullptr || modeSetConsumers.count(module->getId()) == 0)
            throw cRuntimeError("Required mode-set consumer '%s' is not registered", name);
    }
    for (const auto& entry : modeSetConsumers)
        if (getSimulation()->getModule(entry.first) == nullptr)
            throw cRuntimeError("Mode-set consumer was deleted without unregistering");
    pendingModeSet = modeSet;
    changingModeSet = true;
}

void Ieee80211Interface::completeModeSetChange(const Ieee80211ModeSet *modeSet)
{
    Enter_Method_Silent();
    if (!changingModeSet || pendingModeSet != modeSet)
        throw cRuntimeError("Mode-set completion does not match the pending transition");
    for (auto phase : {MAC_STATE, DERIVED_STATE}) {
        for (const auto& entry : modeSetConsumers) {
            if (entry.second != phase)
                continue;
            auto consumer = getSimulation()->getModule(entry.first);
            check_and_cast<IIeee80211ModeSetListener *>(consumer)->applyModeSet(modeSet);
        }
    }
    this->modeSet = modeSet;
    if (modeSetPublished)
        emit(modesetChangedSignal, const_cast<Ieee80211ModeSet *>(modeSet));
    pendingModeSet = nullptr;
    changingModeSet = false;
}

} // namespace ieee80211
} // namespace inet
