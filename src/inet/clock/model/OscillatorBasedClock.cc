//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/OscillatorBasedClock.h"

#include "inet/common/IPrintableObject.h"

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
        useFutureEventSet = par("useFutureEventSet");
        auto oscillatorModule = getSubmodule("oscillator");
        oscillator = check_and_cast<IOscillator *>(oscillatorModule);
        if (!useFutureEventSet)
            oscillatorModule->subscribe(IOscillator::numTicksChangedSignal, this);
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
        WATCH(originSimulationTimeLowerBound);
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
    EV_DEBUG << "Setting clock origin" << EV_FIELD(simulationTime) << EV_FIELD(clockTime) << EV_ENDL;
    ClockCoutIndent indent;
    CLOCK_COUT << "-> setOrigin(" << simulationTime << ", " << clockTime << ")\n";
    // TODO setOrigin can only be set forward in terms of simulation time!
    originSimulationTime = simulationTime;
    originClockTime = clockTime;
    lastClockTime = clockTime; // TODO this should only happen setClockTime but not when the oscillator changes its way of oscillation
    originSimulationTimeLowerBound = oscillator->computeIntervalForTicks(oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin())) + oscillator->getComputationOrigin();
    CLOCK_COUT << "   : " << "originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << std::endl;
    ASSERTCMP(<=, originSimulationTimeLowerBound, originSimulationTime);
    ASSERTCMP(<=, originSimulationTimeLowerBound, simTime());
    ASSERTCMP(>=, originSimulationTimeLowerBound, oscillator->getComputationOrigin());
    ASSERTCMP(==, originSimulationTimeLowerBound, computeSimTimeFromClockTime(originClockTime, true));
    ASSERTCMP(<=, originSimulationTime, simTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    ASSERTCMP(>=, originSimulationTime, computeSimTimeFromClockTime(originClockTime, true));
    ASSERTCMP(<=, originSimulationTime, computeSimTimeFromClockTime(originClockTime, false));
    ASSERTCMP(==, originClockTime, computeClockTimeFromSimTime(originSimulationTime));
    CLOCK_COUT << "   setOrigin() -> void\n";
}

clocktime_t OscillatorBasedClock::doComputeClockTimeFromSimTime(simtime_t simulationTime) const
{
    ClockCoutIndent indent;
    CLOCK_COUT << "-> computeClockTimeFromSimTime(" << simulationTime << ")\n";
    CLOCK_COUT << "   : " << "originSimulationTime = " << originSimulationTime << ", "
                          << "originClockTime = " << originClockTime << ", "
                          << "oscillatorComputationOrigin = " << oscillator->getComputationOrigin() << ", "
                          << "oscillatorCompensation = " << getOscillatorCompensation() << std::endl;
    // time dilation between clock origin and simulationTime
    // TODO: revive compensation    int64_t numTicksFromOscillatorOriginToSimulationTime = oscillator->computeTicksForInterval((simulationTime - originSimulationTime) * (1 + getOscillatorCompensation().get<unit>()) + (originSimulationTime - oscillator->getComputationOrigin()));
    int64_t numTicksFromOscillatorOriginToSimulationTime = oscillator->computeTicksForInterval(simulationTime - oscillator->getComputationOrigin());

    int64_t numTicksFromOscillatorOriginToClockOrigin = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    int64_t numTicks = numTicksFromOscillatorOriginToSimulationTime - numTicksFromOscillatorOriginToClockOrigin;
    ASSERTCMP(>=, numTicks, 0);
    clocktime_t clockTimeFromClockOrigin = SIMTIME_AS_CLOCKTIME(numTicks * oscillator->getNominalTickLength());
    clocktime_t result = originClockTime + clockTimeFromClockOrigin;
    CLOCK_COUT << "   : " << "numTicksFromOscillatorOriginToSimulationTime = " << numTicksFromOscillatorOriginToSimulationTime << ", "
                          << "numTicksFromOscillatorOriginToClockOrigin = " << numTicksFromOscillatorOriginToClockOrigin << ", "
                          << "numTicks = " << numTicks << ", "
                          << "clockTimeFromClockOrigin = " << clockTimeFromClockOrigin << std::endl;
    CLOCK_COUT << "   computeClockTimeFromSimTime(" << simulationTime << ") -> " << result << std::endl;
    return result;
}

simtime_t OscillatorBasedClock::doComputeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    ClockCoutIndent indent;
    CLOCK_COUT << "-> computeSimTimeFromClockTime(" << clockTime << ", " << (lowerBound ? "true" : "false") << ")\n";
    CLOCK_COUT << "   : " << "originSimulationTime = " << originSimulationTime << ", "
                          << "originClockTime = " << originClockTime << ", "
                          << "oscillatorComputationOrigin = " << oscillator->getComputationOrigin() << ", "
                          << "oscillatorCompensation = " << getOscillatorCompensation() << std::endl;
    // TODO but clockTime is not necessarily a multiple of nominalTickLength?! some rounding?
    int64_t numTicksFromClockOriginToClockTime = (clockTime - originClockTime).raw() / oscillator->getNominalTickLength().raw();
    int64_t numTicksFromOscillatorOriginToClockOrigin = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    int64_t numTicks = numTicksFromClockOriginToClockTime +
                       numTicksFromOscillatorOriginToClockOrigin +
                       (lowerBound ? 0 : 1);
    ASSERTCMP(>=, numTicks, 0);
    simtime_t simulationTimeInterval = oscillator->computeIntervalForTicks(numTicks);
    simtime_t result = oscillator->getComputationOrigin() + simulationTimeInterval;
    // time dilation between clock origin and clockTime
    // TODO: revive compensation simtime_t result = originSimulationTime + (oscillator->getComputationOrigin() + simulationTimeInterval - originSimulationTime) / (1 + getOscillatorCompensation().get<unit>());
    CLOCK_COUT << "   : " << "numTicksFromClockOriginToClockTime = " << numTicksFromClockOriginToClockTime << ", "
                          << "numTicksFromOscillatorOriginToClockOrigin = " << numTicksFromOscillatorOriginToClockOrigin << ", "
                          << "numTicks = " << numTicks << std::endl;
    CLOCK_COUT << "   computeSimTimeFromClockTime(" << clockTime << ", " << (lowerBound ? "true" : "false") << ") -> " << result << std::endl;
    return result;
}

clocktime_t OscillatorBasedClock::computeClockTimeFromSimTime(simtime_t simulationTime) const
{
    ASSERTCMP(>=, simulationTime, simTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    clocktime_t result = doComputeClockTimeFromSimTime(simulationTime);
    ASSERTCMP(>=, result, doComputeClockTimeFromSimTime(simTime()));
    ASSERTCMP(>=, simulationTime, doComputeSimTimeFromClockTime(result, true));
    ASSERTCMP(<, simulationTime, doComputeSimTimeFromClockTime(result, false));
    return result;
}

simtime_t OscillatorBasedClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    ASSERTCMP(>=, clockTime, getClockTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    simtime_t result = doComputeSimTimeFromClockTime(clockTime, lowerBound);
    ASSERTCMP(>=, result, originSimulationTimeLowerBound);
    if (lowerBound)
        ASSERTCMP(==, clockTime, doComputeClockTimeFromSimTime(result))
    else
        ASSERTCMP(==, clockTime + SIMTIME_AS_CLOCKTIME(oscillator->getNominalTickLength()), doComputeClockTimeFromSimTime(result));
    return result;
}

void OscillatorBasedClock::scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event)
{
    if (useFutureEventSet || event->getArrivalClockTime() == getClockTime())
        ClockBase::scheduleTargetModuleClockEventAt(time, event);
    else
        event->setArrival(getTargetModule()->getId(), -1);
}

void OscillatorBasedClock::scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event)
{
    if (useFutureEventSet || event->getArrivalClockTime() == getClockTime())
        ClockBase::scheduleTargetModuleClockEventAfter(time, event);
    else
        event->setArrival(getTargetModule()->getId(), -1);
}

ClockEvent *OscillatorBasedClock::cancelTargetModuleClockEvent(ClockEvent *event)
{
    if (useFutureEventSet || event->getArrivalClockTime() == getClockTime())
        return ClockBase::cancelTargetModuleClockEvent(event);
    else
        return event;
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

void OscillatorBasedClock::receiveSignal(cComponent *source, int signal, uintval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == IOscillator::numTicksChangedSignal) {
        clocktime_t clockTime = getClockTime();
        EV_DEBUG << "Handling tick signal" << EV_FIELD(clockTime) << EV_ENDL;
        if (value != lastNumTicks) {
            ASSERT(value - lastNumTicks == 1);
            clockTimeCompensation += SIMTIME_AS_CLOCKTIME(oscillator->getNominalTickLength() * (1 + getOscillatorCompensation().get<unit>()));
            int64_t numTicks = clockTimeCompensation.raw() / oscillator->getNominalTickLength().raw();
            clocktime_t clockTimeDelta = SIMTIME_AS_CLOCKTIME(numTicks * oscillator->getNominalTickLength());
            clockTimeCompensation -= clockTimeDelta;
            setOrigin(simTime(), originClockTime + clockTimeDelta);
            lastNumTicks = value;
        }
        clockTime = getClockTime();
        std::make_heap(events.begin(), events.end(), compareClockEvents);
        while (!events.empty() && events.front()->getArrivalClockTime() <= clockTime)
        {
            std::pop_heap(events.begin(), events.end(), compareClockEvents);
            auto event = events.back();
            events.erase(std::remove(events.begin(), events.end(), event), events.end());
            event->setClock(nullptr);
            cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
            cContextSwitcher contextSwitcher(targetModule);
            EV_DEBUG << "Executing event on oscillator tick" << EV_FIELD(event) << EV_FIELD(clockTime) << EV_ENDL;
            event->execute();
        }
        checkAllClockEvents();
    }
}

void OscillatorBasedClock::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (useFutureEventSet) {
        if (signal == IOscillator::preOscillatorStateChangedSignal) {
            clocktime_t clockTime = getClockTime();
            EV_DEBUG << "Handling pre-oscillator state changed signal" << EV_FIELD(clockTime) << EV_ENDL;
            checkAllClockEvents();
            setOrigin(simTime(), clockTime);
            checkAllClockEvents();
        }
        else if (signal == IOscillator::postOscillatorStateChangedSignal) {
            clocktime_t clockTime = getClockTime();
            EV_DEBUG << "Handling post-oscillator state changed signal" << EV_FIELD(clockTime) << EV_ENDL;
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
            emit(timeChangedSignal, CLOCKTIME_AS_SIMTIME(clockTime));
        }
    }
}

} // namespace inet

