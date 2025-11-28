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
    return t;
}

OscillatorBasedClock::~OscillatorBasedClock()
{
    for (auto event : events)
        event->setClock(nullptr);
}

clocktime_t OscillatorBasedClock::getClockTime() const
{
    clocktime_t currentClockTime = ClockBase::getClockTime();
    ASSERT(currentClockTime.raw() % oscillator->getNominalTickLength().raw() == 0);
    return currentClockTime;
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
        WATCH(originSimulationTime);
        WATCH(originClockTime);
        WATCH_PTRVECTOR(events);
    }
    else if (stage == INITSTAGE_CLOCK) {
        setOrigin(simTime(), par("initialClockTime"));
        if (originClockTime.raw() % oscillator->getNominalTickLength().raw() != 0)
            throw cRuntimeError("Initial clock time must be a multiple of the oscillator nominal tick length");
    }
}

void OscillatorBasedClock::setOrigin(simtime_t simulationTime, clocktime_t clockTime)
{
    originSimulationTime = simulationTime;
    originClockTime = clockTime;
    lastClockTime = clockTime;
    ASSERTCMP(<=, originSimulationTime, simTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    ASSERTCMP(>=, originSimulationTime, computeSimTimeFromClockTime(originClockTime, true));
    ASSERTCMP(<=, originSimulationTime, computeSimTimeFromClockTime(originClockTime, false));
    ASSERTCMP(==, originClockTime, computeClockTimeFromSimTime(originSimulationTime));
}

clocktime_t OscillatorBasedClock::computeClockTimeFromSimTime(simtime_t simulationTime) const
{
    ASSERT(simulationTime >= simTime());
    ASSERT(originSimulationTime >= oscillator->getComputationOrigin());
    int64_t numTicksFromOscillatorOriginToSimulationTime = oscillator->computeTicksForInterval(simulationTime - oscillator->getComputationOrigin());
    int64_t numTicksFromOscillatorOriginToClockOrigin = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    int64_t numTicks = numTicksFromOscillatorOriginToSimulationTime - numTicksFromOscillatorOriginToClockOrigin;
    clocktime_t clockTimeFromClockOrigin = SIMTIME_AS_CLOCKTIME(numTicks * oscillator->getNominalTickLength() * (1 + getOscillatorCompensation().get<unit>()));
    clocktime_t result = originClockTime + clockTimeFromClockOrigin;
    return result;
}

simtime_t OscillatorBasedClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    ASSERT(clockTime >= getClockTime());
    ASSERT(originSimulationTime >= oscillator->getComputationOrigin());
    int64_t numTicksFromClockOriginToClockTime = (clockTime - originClockTime).raw() / oscillator->getNominalTickLength().raw() / (1 + getOscillatorCompensation().get<unit>());
    int64_t numTicksFromOscillatorOriginToClockOrigin = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    int64_t numTicks = numTicksFromClockOriginToClockTime +
                       numTicksFromOscillatorOriginToClockOrigin +
                       (lowerBound ? 0 : 1);
    simtime_t result = oscillator->getComputationOrigin() + oscillator->computeIntervalForTicks(numTicks);
    return result;
}

void OscillatorBasedClock::scheduleClockEventAt(clocktime_t time, ClockEvent *event)
{
    ASSERTCMP(>=, time, getClockTime());
    int64_t roundedTime = roundingFunction(time.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAt(ClockTime().setRaw(roundedTime), event);
    events.push_back(event);
    checkClockEvent(event);
}

void OscillatorBasedClock::scheduleClockEventAfter(clocktime_t delay, ClockEvent *event)
{
    ASSERTCMP(>=, delay, 0);
    int64_t roundedDelay = roundingFunction(delay.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAfter(ClockTime().setRaw(roundedDelay), event);
    events.push_back(event);
    checkClockEvent(event);
}

ClockEvent *OscillatorBasedClock::cancelClockEvent(ClockEvent *event)
{
    checkClockEvent(event);
    events.erase(std::remove(events.begin(), events.end(), event), events.end());
    return ClockBase::cancelClockEvent(event);
}

void OscillatorBasedClock::handleClockEvent(ClockEvent *event)
{
    events.erase(std::remove(events.begin(), events.end(), event), events.end());
    ClockBase::handleClockEvent(event);
}

std::string OscillatorBasedClock::resolveDirective(char directive) const
{
    switch (directive) {
        case 's':
            return originSimulationTime.str();
        case 'c':
            return originClockTime.str();
        default:
            return ClockBase::resolveDirective(directive);
    }
}

void OscillatorBasedClock::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IOscillator::preOscillatorStateChangedSignal) {
        checkAllClockEvents();
        setOrigin(simTime(), getClockTime());
        checkAllClockEvents();
    }
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
            checkClockEvent(event);
        }
        emit(timeChangedSignal, CLOCKTIME_AS_SIMTIME(getClockTime()));
    }
}

} // namespace inet

