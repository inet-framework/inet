//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/base/ClockBase.h"
#include "inet/common/ModuleRefByPar.h"

namespace inet {

simsignal_t ClockBase::timeChangedSignal = cComponent::registerSignal("timeChanged");

void ClockBase::initialize(int stage)
{
    SimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        emitClockTimeInterval = par("emitClockTimeInterval");
        if (emitClockTimeInterval != 0) {
            timer = new cMessage();
            scheduleAt(simTime(), timer);
        }
    }
    else if (stage == INITSTAGE_LAST) {
        referenceClockModule.reference(this, "referenceClock", false);
        emit(timeChangedSignal, getClockTime().asSimTime());
    }
}

void ClockBase::handleMessage(cMessage *msg)
{
    if (msg == timer) {
        emit(timeChangedSignal, getClockTime().asSimTime());
        scheduleAfter(emitClockTimeInterval, timer);
    }
    else
        throw cRuntimeError("Unknown message");
}

void ClockBase::finish()
{
    emit(timeChangedSignal, getClockTime().asSimTime());
}


clocktime_t ClockBase::getClockTime() const
{
    return computeClockTimeFromSimTime(simTime());
}

simtime_t ClockBase::computeScheduleTime(clocktime_t clockTime)
{
    simtime_t currentSimulationTime = simTime();
    simtime_t lowerSimulationTime = computeSimTimeFromClockTime(clockTime, true);
    if (lowerSimulationTime >= currentSimulationTime)
        return lowerSimulationTime;
    else {
        simtime_t upperSimulationTime = computeSimTimeFromClockTime(clockTime, false);
        if (currentSimulationTime < upperSimulationTime)
            return currentSimulationTime;
        else
            return lowerSimulationTime;
    }
}

void ClockBase::scheduleClockEventAt(clocktime_t t, ClockEvent *msg)
{
    if (t < getClockTime())
        throw cRuntimeError("Cannot schedule clock event in the past");
    cSimpleModule *targetModule = getTargetModule();
    msg->setClock(this);
    msg->setRelative(false);
    msg->setArrivalClockTime(t);
    targetModule->scheduleAt(computeScheduleTime(t), msg);
}

void ClockBase::scheduleClockEventAfter(clocktime_t clockTimeDelay, ClockEvent *msg)
{
    if (clockTimeDelay < 0)
        throw cRuntimeError("Cannot schedule clock event with negative delay");
    cSimpleModule *targetModule = getTargetModule();
    msg->setClock(this);
    msg->setRelative(true);
    clocktime_t nowClock = getClockTime();
    clocktime_t arrivalClockTime = nowClock + clockTimeDelay;
    msg->setArrivalClockTime(arrivalClockTime);
    simtime_t simTimeDelay = clockTimeDelay.isZero() ? SIMTIME_ZERO : computeScheduleTime(arrivalClockTime) - simTime();
    targetModule->scheduleAfter(simTimeDelay, msg);
}

ClockEvent *ClockBase::cancelClockEvent(ClockEvent *msg)
{
    getTargetModule()->cancelEvent(msg);
    msg->setClock(nullptr);
    return msg;
}

void ClockBase::handleClockEvent(ClockEvent *msg)
{
    msg->setClock(nullptr);
    msg->callBaseExecute();
}

std::string ClockBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 't':
            return getClockTime().str() + " s";
        case 'T':
            return getClockTime().ustr();
        case 'd':
            return (getClockTime() - referenceClockModule->getClockTime()).ustr();
        default:
            return SimpleModule::resolveDirective(directive);
    }
}

} // namespace inet

