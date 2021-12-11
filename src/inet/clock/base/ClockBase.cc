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
    if (stage == INITSTAGE_LOCAL)
        displayStringTextFormat = par("displayStringTextFormat");
    else if (stage == INITSTAGE_LAST) {
        referenceClockModule.reference(this, "referenceClock", false);
        updateDisplayString();
        emit(timeChangedSignal, getClockTime().asSimTime());
    }
}

void ClockBase::finish()
{
    emit(timeChangedSignal, getClockTime().asSimTime());
}

void ClockBase::refreshDisplay() const
{
    updateDisplayString();
}

void ClockBase::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(displayStringTextFormat, this);
        getDisplayString().setTagArg("t", 0, text);
    }
}

clocktime_t ClockBase::getClockTime() const
{
    return clockEventTime != -1 ? clockEventTime : computeClockTimeFromSimTime(simTime());
}

void ClockBase::scheduleClockEventAt(clocktime_t t, ClockEvent *msg)
{
    if (t < getClockTime())
        throw cRuntimeError("Cannot schedule clock event in the past");
    cSimpleModule *targetModule = getTargetModule();
    msg->setClock(this);
    msg->setRelative(false);
    msg->setArrivalClockTime(t);
    targetModule->scheduleAt(computeSimTimeFromClockTime(t), msg);
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
    simtime_t simTimeDelay = clockTimeDelay.isZero() ? SIMTIME_ZERO : computeSimTimeFromClockTime(arrivalClockTime) - simTime();
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
    clockEventTime = msg->getArrivalClockTime();
    msg->setClock(nullptr);
    msg->callBaseExecute();
    clockEventTime = -1;
}

const char *ClockBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 't':
            result = getClockTime().str() + " s";
            break;
        case 'T':
            result = getClockTime().ustr();
            break;
        case 'd':
            result = (getClockTime() - referenceClockModule->getClockTime()).ustr();
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

} // namespace inet

