//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "HotStandby.h"

#include "Gptp.h"

namespace inet {

Define_Module(HotStandby);

// TODO: Fix: Domain mumber isn't neccessarily the same as the clock index
void HotStandby::initialize(int stage)
{
    ClockUserModuleBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        auto parent = getParentModule();
        auto numDomains = parent->par("numDomains").intValue();

        multiClock = check_and_cast<MultiClock *>(clock.get());

        for (int i = 0; i < numDomains; i++) {
            auto gptp = check_and_cast<Gptp *>(parent->getSubmodule("domain", i));
            gptp->subscribe(Gptp::gptpSyncStateChanged, this);
            ModuleRefByPar<ClockBase> currentClock;
            currentClock.reference(gptp, "clockModule", true);
            auto clockParent = currentClock->getParentModule();
            if (clockParent != multiClock) {
                EV_WARN
                    << "Clock module is not a child of MultiClock, cannot determine clock index and ignoring this clock"
                    << endl;
            }
            else {
                domainNumberToClockIndex[gptp->getDomainNumber()] = currentClock->getIndex();
            }
        }
    }
}

void HotStandby::receiveSignal(cComponent *source, simsignal_t simSignal, const intval_t t, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simSignal));
    if (simSignal == Gptp::gptpSyncStateChanged) {
        auto gptp = check_and_cast<const Gptp *>(source);
        auto syncState = static_cast<SyncState>(t);
        handleSyncStateChanged(gptp, syncState);
    }
}

void HotStandby::handleSyncStateChanged(const Gptp *gptp, const SyncState syncState)
{
    // store sync state into array
    auto domainNumber = gptp->getDomainNumber();
    syncStates[domainNumber] = syncState;

    auto currentActiveIndex = multiClock->par("activeClockIndex").intValue();
    // TODO: We assume lower domain is always preferable
    //  In the future, domain preference should be configurable
    if (domainNumber > currentActiveIndex || (domainNumber == currentActiveIndex && syncState == SYNCED)) {
        // if the domain is higher than the current active domain, ignore it
        return;
    }

    int bestDomainNumber = std::numeric_limits<int>::max();
    for (auto &entry : syncStates) {
        if (entry.second == SYNCED && entry.first < bestDomainNumber) {
            bestDomainNumber = entry.first;
        }
    }

    if (bestDomainNumber == std::numeric_limits<int>::max()) {
        EV_WARN << "All domains out of sync, cannot switch to backup domain" << endl;
        return;
    }

    auto clockIndex = domainNumberToClockIndex[bestDomainNumber];

    EV_INFO << "Switching to domain " << bestDomainNumber << " - clock index " << clockIndex << endl;
    multiClock->par("activeClockIndex").setIntValue(clockIndex);
}

} // namespace inet
