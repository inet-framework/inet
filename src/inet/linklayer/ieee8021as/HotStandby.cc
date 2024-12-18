//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "HotStandby.h"



namespace inet {

Define_Module(HotStandby);

HotStandby::~HotStandby()
{
    if(standByMsg)
        cancelAndDeleteClockEvent(standByMsg);
}

void HotStandby::initialize(int stage)
{
    ClockUserModuleBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        auto parent = getParentModule();
        for (int i = 0; i < parent->getSubmoduleVectorSize("domain"); i++) {
            auto gptp = parent->getSubmodule("domain", i);
            gptp->subscribe(Gptp::gptpSyncSuccessfulSignal, this);
        }
        gptpModule = check_and_cast<const Gptp *>(parent->getSubmodule("domain", 0));
        standByMsg = new ClockEvent("standByMsg", GPTP_CLOCK_DOWN_MSG);

        delta = par("delta");
        scheduleClockEventAfter(1, standByMsg); // set clocktime_t as delta, timer or 0 won't work. Why??
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

    auto syncInterval = gptp->getSyncInterval();
    timer = 2 * syncInterval + delta;
    rescheduleClockEventAfter(timer, standByMsg);
    EV_INFO << "timer : " << timer << endl;
}

void HotStandby::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage() && msg->getKind() == GPTP_CLOCK_DOWN_MSG){
        ASSERT(standByMsg == msg);


//        // check the time difference between the domains -- TODO: Fix the logic of judgement for timeDiff
//        auto domainNumber = gptpModule->getDomainNumber();
//        auto domainClockTime = gptpSyncTime[domainNumber];
//        auto curr = getClockTime();
//        auto timeDiff = curr - domainClockTime;
//        EV_INFO << "domain number : " << domainNumber << std::endl;
//        EV_INFO << "domain clock time : " << domainClockTime << std::endl;
//        EV_INFO << "current time : " << curr << std::endl;
//        EV_INFO << "timeDiff : " << timeDiff << std::endl;
//
//        if(timeDiff >= timer){
//            EV_INFO << "timeDiff is greater than timer" << std::endl;
//            // reschedule the timer for the next clock down event
//            EV_INFO << "TIMER : " << timer << std::endl;
//            rescheduleClockEventAfter(timer, standByMsg);
//        }
    }
    else {
        throw cRuntimeError("Unknown self message (%s)%s, kind=%d", msg->getClassName(), msg->getName(), msg->getKind());
    }
}

} // namespace inet
