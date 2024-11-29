//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "HotStandby.h"



namespace inet {

Define_Module(HotStandby);

void HotStandby::initialize(int stage)
{
    ClockUserModuleBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        auto parent = getParentModule();
        for (int i = 0; i < parent->getSubmoduleVectorSize("domain"); i++) {
            auto gptp = parent->getSubmodule("domain", i);
            gptp->subscribe(Gptp::gptpSyncSuccessfulSignal, this);
        }
    }
}

void HotStandby::receiveSignal(cComponent *source, simsignal_t simSignal, const SimTime& t, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simSignal));
    if (simSignal == Gptp::gptpSyncSuccessfulSignal) {
        auto gptp = check_and_cast<const Gptp *>(source);
        handleGptpSyncSuccessfulSignal(gptp, t);
    }
}

void HotStandby::handleGptpSyncSuccessfulSignal(const Gptp *gptp, const SimTime& t)
{
    // store time into array
    auto domainNumber = gptp->getDomainNumber();
    auto clockTime = SIMTIME_AS_CLOCKTIME(t);
    gptpSyncTime[domainNumber] = clockTime;
}

} // namespace inet
