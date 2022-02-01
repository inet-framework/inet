//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/OscillatorBasedClock.h"

#include <algorithm>

namespace inet {

Define_Module(OscillatorBasedClock);

static int64_t roundUp(int64_t t, int64_t l)
{
    return (t + l - 1) / l * l;
}

static int64_t roundDown(int64_t t, int64_t l)
{
    return (t / l) * l;
}

static int64_t roundCloser(int64_t t, int64_t l)
{
    return (t + l / 2) / l * l;
}

static int64_t roundNone(int64_t t, int64_t l)
{
    if (t % l != 0)
        throw cRuntimeError("Clock time/delay value is not multiple of nominal tick length");
    return t / l;
}

OscillatorBasedClock::~OscillatorBasedClock()
{
    for (auto event : events)
        event->setClock(nullptr);
}

void OscillatorBasedClock::initialize(int stage)
{
    ClockBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        auto oscillatorModule = getSubmodule("oscillator");
        oscillator = check_and_cast<IOscillator *>(oscillatorModule);
        oscillatorModule->subscribe(IOscillator::preOscillatorStateChangedSignal, this);
        oscillatorModule->subscribe(IOscillator::postOscillatorStateChangedSignal, this);
        const char *roundingMode = par("roundingMode");
        if (!strcmp(roundingMode, "up"))
            roundingFunction = roundUp;
        else if (!strcmp(roundingMode, "down"))
            roundingFunction = roundDown;
        else if (!strcmp(roundingMode, "closer"))
            roundingFunction = roundCloser;
        else if (!strcmp(roundingMode, "none"))
            roundingFunction = roundNone;
        else
            throw cRuntimeError("Unknown rounding mode");
        WATCH(originClockTick);
        WATCH_PTRVECTOR(events);
    }
    else if (stage == INITSTAGE_CLOCK) {
        simtime_t initialClockTime = par("initialClockTime");
        if (initialClockTime.raw() % oscillator->getNominalTickLength().raw() != 0)
            throw cRuntimeError("Initial clock time must be a multiple of the oscillator nominal tick length");
        originClockTick = initialClockTime.raw() / oscillator->getNominalTickLength().raw();
    }
}

clocktime_t OscillatorBasedClock::computeClockTimeFromSimTime(simtime_t t) const
{
    ASSERT(t >= simTime());
    return ClockTime::from((originClockTick + oscillator->computeTicksForInterval(t - oscillator->getComputationOrigin())) * oscillator->getNominalTickLength());
}

simtime_t OscillatorBasedClock::computeSimTimeFromClockTime(clocktime_t t) const
{
    ASSERT(t >= getClockTime());
    int64_t numTicks = t.raw() / oscillator->getNominalTickLength().raw();
    return oscillator->getComputationOrigin() + oscillator->computeIntervalForTicks(numTicks - originClockTick);
}

void OscillatorBasedClock::scheduleClockEventAt(clocktime_t time, ClockEvent *event)
{
    int64_t roundedTime = roundingFunction(time.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAt(ClockTime().setRaw(roundedTime), event);
    events.push_back(event);
}

void OscillatorBasedClock::scheduleClockEventAfter(clocktime_t delay, ClockEvent *event)
{
    int64_t roundedDelay = roundingFunction(delay.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAfter(ClockTime().setRaw(roundedDelay), event);
    events.push_back(event);
}

ClockEvent *OscillatorBasedClock::cancelClockEvent(ClockEvent *event)
{
    events.erase(std::remove(events.begin(), events.end(), event), events.end());
    return ClockBase::cancelClockEvent(event);
}

void OscillatorBasedClock::handleClockEvent(ClockEvent *event)
{
    events.erase(std::remove(events.begin(), events.end(), event), events.end());
    ClockBase::handleClockEvent(event);
}

const char *OscillatorBasedClock::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'o':
            result = std::to_string(originClockTick);
            break;
        case 'c':
            result = std::to_string(originClockTick + oscillator->computeTicksForInterval(simTime() - oscillator->getComputationOrigin()));
            break;
        default:
            return ClockBase::resolveDirective(directive);
    }
    return result.c_str();
}

void OscillatorBasedClock::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IOscillator::preOscillatorStateChangedSignal)
        originClockTick += oscillator->computeTicksForInterval(simTime() - oscillator->getComputationOrigin());
    else if (signal == IOscillator::postOscillatorStateChangedSignal) {
        simtime_t currentSimTime = simTime();
        for (auto event : events) {
            if (event->getRelative()) {
                simtime_t simTimeDelay = computeSimTimeFromClockTime(event->getArrivalClockTime()) - currentSimTime;
                ASSERT(simTimeDelay >= 0);
                cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                cContextSwitcher contextSwitcher(targetModule);
                targetModule->rescheduleAfter(simTimeDelay, event);
            }
            else {
                clocktime_t arrivalClockTime = event->getArrivalClockTime();
                simtime_t arrivalSimTime = computeSimTimeFromClockTime(arrivalClockTime);
                ASSERT(arrivalSimTime >= currentSimTime);
                cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                cContextSwitcher contextSwitcher(targetModule);
                targetModule->rescheduleAt(arrivalSimTime, event);
            }
        }
        emit(timeChangedSignal, CLOCKTIME_AS_SIMTIME(getClockTime()));
    }
}

} // namespace inet

