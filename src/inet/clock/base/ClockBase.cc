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
            timer = new cMessage("ClockTimeChangedTimer");
            timer->setSchedulingPriority(par("clockTimeChangeEventSchedulingPriority"));
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

void ClockBase::checkScheduledClockEvent(const ClockEvent *event) const
{
    DEBUG_ENTER();
    // NOTE: IClock interface 3. invariant
    DEBUG_CMP(event->getArrivalClockTime(), >=, getClockTime());
    if (event->isScheduled()) {
        DEBUG_CMP(event->getArrivalTime(), >=, simTime());
        // NOTE: IClock interface 4. invariant
        DEBUG_CMP(event->getArrivalTime(), ==, computeScheduleTime(event->getArrivalClockTime()));
        // NOTE: IClock interface 5. invariant
//        DEBUG_CMP(event->getArrivalClockTime(), >=, computeClockTimeFromSimTime(event->getArrivalTime(), false));
//        DEBUG_CMP(event->getArrivalClockTime(), <=, computeClockTimeFromSimTime(event->getArrivalTime(), true));
    }
    DEBUG_LEAVE();
}

cSimpleModule *ClockBase::getTargetModule() const
{
    cSimpleModule *target = getSimulation()->getContextSimpleModule();
    if (target == nullptr)
        throw cRuntimeError("scheduleAt()/cancelEvent() must be called with a simple module in context");
    return target;
}

clocktime_t ClockBase::getClockTime() const
{
    return clockEventTime != -1 ? clockEventTime : computeClockTimeFromSimTime(simTime());
}

simtime_t ClockBase::computeScheduleTime(clocktime_t clockTime) const
{
    DEBUG_ENTER(clockTime);
    simtime_t currentSimulationTime = simTime();
    simtime_t lowerSimulationTime = computeSimTimeFromClockTime(clockTime, true);
    DEBUG_OUT << DEBUG_FIELD(lowerSimulationTime) << std::endl;
    simtime_t result;
    if (lowerSimulationTime >= currentSimulationTime)
        result = lowerSimulationTime;
    else {
        simtime_t upperSimulationTime = computeSimTimeFromClockTime(clockTime, false);
        DEBUG_OUT << DEBUG_FIELD(upperSimulationTime) << std::endl;
        if (currentSimulationTime < upperSimulationTime)
            result = currentSimulationTime;
        else
            result = lowerSimulationTime; // NOTE: upperSimulationTime is exclusive
    }
    DEBUG_LEAVE(result);
    return result;
}

void ClockBase::scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *msg)
{
    cSimpleModule *targetModule = getTargetModule();
    targetModule->scheduleAt(time, msg);
}

void ClockBase::scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *msg)
{
    cSimpleModule *targetModule = getTargetModule();
    targetModule->scheduleAfter(time, msg);
}

ClockEvent *ClockBase::cancelTargetModuleClockEvent(ClockEvent *msg)
{
    cSimpleModule *targetModule = getTargetModule();
    targetModule->cancelEvent(msg);
    return msg;
}

void ClockBase::scheduleClockEventAt(clocktime_t t, ClockEvent *msg)
{
    if (t < getClockTime())
        throw cRuntimeError("Cannot schedule clock event in the past");
    msg->setClock(this);
    msg->setRelative(false);
    msg->setArrivalClockTime(t);
    scheduleTargetModuleClockEventAt(computeScheduleTime(t), msg);
    checkScheduledClockEvent(msg);
}

void ClockBase::scheduleClockEventAfter(clocktime_t clockTimeDelay, ClockEvent *msg)
{
    if (clockTimeDelay < 0)
        throw cRuntimeError("Cannot schedule clock event with negative delay");
    msg->setClock(this);
    msg->setRelative(true);
    clocktime_t nowClock = getClockTime();
    clocktime_t arrivalClockTime = nowClock + clockTimeDelay;
    msg->setArrivalClockTime(arrivalClockTime);
    simtime_t simTimeDelay = clockTimeDelay.isZero() ? SIMTIME_ZERO : computeScheduleTime(arrivalClockTime) - simTime();
    scheduleTargetModuleClockEventAfter(simTimeDelay, msg);
    checkScheduledClockEvent(msg);
}

ClockEvent *ClockBase::cancelClockEvent(ClockEvent *msg)
{
    cancelTargetModuleClockEvent(msg);
    msg->setClock(nullptr);
    return msg;
}

void ClockBase::handleClockEvent(ClockEvent *msg)
{
    clockEventTime = msg->getArrivalClockTime();
    msg->setClock(nullptr);
    msg->callBaseExecute();
    clockEventTime = -1;
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

