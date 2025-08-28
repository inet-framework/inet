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

static simtime_raw_t roundUp(simtime_raw_t t, simtime_raw_t l)
{
    return (t + l - 1) / l * l;
}

static simtime_raw_t roundDown(simtime_raw_t t, simtime_raw_t l)
{
    return (t / l) * l;
}

static simtime_raw_t roundCloser(simtime_raw_t t, simtime_raw_t l)
{
    return (t + l / 2) / l * l;
}

static simtime_raw_t roundNone(simtime_raw_t t, simtime_raw_t l)
{
    if (t % l != 0)
        throw cRuntimeError("Clock time/delay value is not multiple of nominal tick length");
    return t;
}

OscillatorBasedClock::~OscillatorBasedClock()
{
//    for (auto event : events)
//        event->setClock(nullptr);
    cancelAndDelete(executeEventsTimer);
}

clocktime_t OscillatorBasedClock::getClockTime() const
{
    if (useFutureEventSet) {
        clocktime_t currentClockTime = ClockBase::getClockTime();
        ASSERT(currentClockTime.raw() % oscillator->getNominalTickLength().raw() == 0);
        return currentClockTime;
    }
    else {
        // NOTE: IClock interface 1. invariant
        ASSERTCMP(>=, originClockTime, lastClockTime);
        lastClockTime = originClockTime;
        return originClockTime;
    }
}

void OscillatorBasedClock::initialize(int stage)
{
    ClockBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        useFutureEventSet = par("useFutureEventSet");
        oscillator = getModuleFromPar<IOscillator>(par("oscillatorModule"), this);
        auto oscillatorModule = check_and_cast<cModule *>(oscillator);
        if (!useFutureEventSet) {
            oscillatorModule->subscribe(IOscillator::numTicksChangedSignal, this);
            executeEventsTimer = new cMessage("ExecuteClockEventsTimer");
        }
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

void OscillatorBasedClock::handleMessage(cMessage *message)
{
    if (message == executeEventsTimer) {
        clocktime_t clockTime = getClockTime();
        std::make_heap(events.begin(), events.end(), compareClockEvents);
        while (!events.empty() && events.front()->getArrivalClockTime() <= clockTime)
        {
            std::pop_heap(events.begin(), events.end(), compareClockEvents);
            auto event = events.back();
            events.pop_back();
            event->setClock(nullptr);
            cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
            cContextSwitcher contextSwitcher(targetModule);
            EV_DEBUG << "Executing clock event" << EV_FIELD(clockTime) << EV_FIELD(event) << EV_ENDL;
            event->execute();
        }
        checkAllClockEvents();
    }
    else
        ClockBase::handleMessage(message);
}

int64_t OscillatorBasedClock::A(int64_t n) const {
    double x = getOscillatorCompensation().toDouble();
    return floor(p + (x - 1) * n);
}

int64_t OscillatorBasedClock::F(int64_t n) const {
    return n + A(n);
}

// TODO: calling this function may or may not affect how the clock estimates future clock time vs simulation time
// TODO: this method serves two purposes, differentiate the two, just moving the origin, or updating the clock
void OscillatorBasedClock::setOrigin(simtime_t simulationTime, clocktime_t clockTime)
{
    EV_DEBUG << "Setting clock origin" << EV_FIELD(simulationTime) << EV_FIELD(clockTime) << EV_ENDL;
    ClockCoutIndent indent;
    CLOCK_COUT << "-> setOrigin(" << simulationTime << ", " << clockTime << ")\n";

    // (optional but recommended)
    ASSERTCMP(>=, simulationTime, originSimulationTime);
    ASSERTCMP(>=, simulationTime, oscillator->getComputationOrigin());

    const int64_t numTicksFromOscillatorOriginToSimulationTime = oscillator->computeTicksForInterval(simulationTime - oscillator->getComputationOrigin());
    const simtime_t newOriginSimulationTimeLowerBound = oscillator->getComputationOrigin() + oscillator->computeIntervalForTicks(numTicksFromOscillatorOriginToSimulationTime);

    // mathematical definitions
    int64_t l = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const simtime_t& s = simulationTime;
    const clocktime_t& c = clockTime;
    double x = getOscillatorCompensation().toDouble();
    auto numTicks = [=] (auto interval) { return oscillator->computeTicksForInterval(interval); };
    auto frac = [] (double y) { double f = std::floor(y); return y - f; };
    // mathematical formula
    int64_t n0_old = numTicks(cos - oos);
    int64_t n = numTicks(s - oos);
    simtime_t cos_ = s;
    int64_t coc_ = c.raw() - (F(n) - F(n0_old)) * l;
    p = frac(p + (x - 1.0) * (double)n);

    originSimulationTimeLowerBound = newOriginSimulationTimeLowerBound;
    originSimulationTime = cos_;
    originClockTime = ClockTime::fromRaw(coc_);
    // TODO: Do NOT touch lastClockTime here unless this is a user "set time" op. only affects assertions
    lastClockTime = clockTime;

    CLOCK_COUT << "   : originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << std::endl;

    // Your original consistency checks
    ASSERTCMP(<=, originSimulationTimeLowerBound, originSimulationTime);
    ASSERTCMP(<=, originSimulationTimeLowerBound, simTime());
    ASSERTCMP(<=, oscillator->getComputationOrigin(), originSimulationTimeLowerBound);
    ASSERTCMP(==, originSimulationTimeLowerBound, computeSimTimeFromClockTime(originClockTime, true));
    ASSERTCMP(<=, originSimulationTime, simTime());
    ASSERTCMP(<=, oscillator->getComputationOrigin(), originSimulationTime);
    ASSERTCMP(<=, originSimulationTimeLowerBound, originSimulationTime);
    ASSERTCMP(<=, originSimulationTime, computeSimTimeFromClockTime(originClockTime, false));
    ASSERTCMP(==, originClockTime, computeClockTimeFromSimTime(originSimulationTime));

    CLOCK_COUT << "   setOrigin() -> void\n";
}

clocktime_t OscillatorBasedClock::doComputeClockTimeFromSimTime(simtime_t simulationTime) const
{
    // mathematical definitions
    const int64_t l = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const clocktime_t& coc = originClockTime;
    const simtime_t& s = simulationTime;
    auto numTicks = [=] (auto i) { return oscillator->computeTicksForInterval(i); };
    // mathematical formula
    int64_t n = numTicks(s - oos);
    int64_t n0 = numTicks(cos - oos);
    int64_t c = coc.raw() + (F(n) - F(n0)) * l;
    return ClockTime::fromRaw(c);
}

simtime_t OscillatorBasedClock::doComputeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    // mathematical definitions
    bool b = lowerBound;
    int64_t l = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const clocktime_t& coc = originClockTime;
    const clocktime_t& c = clockTime;
    const double x = getOscillatorCompensation().toDouble();
    auto numTicks = [=] (auto interval) { return oscillator->computeTicksForInterval(interval); };
    auto interval = [=] (auto numTicks) { return oscillator->computeIntervalForTicks(numTicks); };
    // mathematical formula
    int64_t n0 = numTicks(cos - oos);
    int64_t k = floor((c - coc).raw() / l) + (b ? 0 : 1);
    int64_t T = F(n0) + k;
    int64_t n1 = std::max<int64_t>(0, (int64_t)std::floor(((double)T - p) / x));
    simtime_t s = oos + interval(n1);
    return s;
}

clocktime_t OscillatorBasedClock::computeClockTimeFromSimTime(simtime_t simulationTime) const
{
    ASSERTCMP(>=, simulationTime, simTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    clocktime_t result = doComputeClockTimeFromSimTime(simulationTime);
    ASSERTCMP(==, result.raw() % getClockGranularity().raw(), 0);
    ASSERTCMP(>=, result, doComputeClockTimeFromSimTime(simTime()));
    ASSERTCMP(>=, simulationTime, doComputeSimTimeFromClockTime(result, true));
    ASSERTCMP(<, simulationTime, doComputeSimTimeFromClockTime(result, false));
    return result;
}

simtime_t OscillatorBasedClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    ASSERTCMP(>=, clockTime, getClockTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    ASSERTCMP(==, clockTime.raw() % getClockGranularity().raw(), 0);
    simtime_t result = doComputeSimTimeFromClockTime(clockTime, lowerBound);
    ASSERTCMP(>=, result, originSimulationTimeLowerBound); // TODO: more restrictive assert for now?
    if (lowerBound)
        ASSERTCMP(==, clockTime, doComputeClockTimeFromSimTime(result)) // TODO: what happens if due to oscillator compensation a specific clock time value was skipped
//        ASSERTCMP(<=, clockTime, doComputeClockTimeFromSimTime(result)) // maybe this?
    else
        // uses inequality because oscillator compensation
        ASSERTCMP(<=, clockTime + getClockGranularity(), doComputeClockTimeFromSimTime(result));
    return result;
}

void OscillatorBasedClock::scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event)
{
    if (useFutureEventSet)
        ClockBase::scheduleTargetModuleClockEventAt(time, event);
    else {
        event->setArrival(getTargetModule()->getId(), -1);
        if (event->getArrivalClockTime() == getClockTime() && !executeEventsTimer->isScheduled())
            scheduleAfter(0, executeEventsTimer);
    }
}

void OscillatorBasedClock::scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event)
{
    if (useFutureEventSet)
        ClockBase::scheduleTargetModuleClockEventAfter(time, event);
    else {
        event->setArrival(getTargetModule()->getId(), -1);
        if (event->getArrivalClockTime() == getClockTime() && !executeEventsTimer->isScheduled())
            scheduleAfter(0, executeEventsTimer);
    }
}

ClockEvent *OscillatorBasedClock::cancelTargetModuleClockEvent(ClockEvent *event)
{
    if (useFutureEventSet)
        return ClockBase::cancelTargetModuleClockEvent(event);
    else
        return event;
}

void OscillatorBasedClock::scheduleClockEventAt(clocktime_t time, ClockEvent *event)
{
    ASSERTCMP(>=, time, getClockTime());
    simtime_raw_t roundedTime = roundingFunction(time.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAt(ClockTime().setRaw(roundedTime), event);
    events.push_back(event);
    if (!useFutureEventSet)
        event->setInsertionOrder(insertionCount++);
    checkClockEvent(event);
}

void OscillatorBasedClock::scheduleClockEventAfter(clocktime_t delay, ClockEvent *event)
{
    ASSERTCMP(>=, delay, 0);
    simtime_raw_t roundedDelay = roundingFunction(delay.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAfter(ClockTime().setRaw(roundedDelay), event);
    events.push_back(event);
    if (!useFutureEventSet)
        event->setInsertionOrder(insertionCount++);
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

bool OscillatorBasedClock::isScheduledClockEvent(ClockEvent *event) const
{
    return useFutureEventSet ? ClockBase::isScheduledClockEvent(event) : std::find(events.begin(), events.end(), event) != events.end();
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
        originSimulationTime = simTime();
        clocktime_t clockTime = getClockTime();
        EV_DEBUG << "Handling tick signal" << EV_FIELD(clockTime) << EV_ENDL;
        if (value != lastNumTicks) {
            ASSERT(value - lastNumTicks == 1);
            clockTimeCompensation += SIMTIME_AS_CLOCKTIME(oscillator->getNominalTickLength() * getOscillatorCompensation());
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
            events.pop_back();
            event->setClock(nullptr);
            cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
            cContextSwitcher contextSwitcher(targetModule);
            EV_DEBUG << "Executing clock event" << EV_FIELD(clockTime) << EV_FIELD(event) << EV_ENDL;
            event->execute();
        }
        checkAllClockEvents();
    }
}

void OscillatorBasedClock::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == IOscillator::preOscillatorStateChangedSignal) {
        clocktime_t clockTime = getClockTime();
        EV_DEBUG << "Handling pre-oscillator state changed signal" << EV_FIELD(clockTime) << EV_ENDL;
        std::cout << "Before setOrigin() call" << ", originClockTime = " << originClockTime << ", originSimulationTime = " << originSimulationTime << ", originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << ", getOscillatorCompensation() = " << getOscillatorCompensation() << ", oscillator->getComputationOrigin() = " << oscillator->getComputationOrigin() << ", compensationPhaseBaseTicks = " << compensationPhaseBaseTicks << ", numTicksAtOriginLowerBound = " << numTicksAtOriginLowerBound << std::endl;
        checkAllClockEvents();
        setOrigin(simTime(), clockTime);
        std::cout << "After  setOrigin() call" << ", originClockTime = " << originClockTime << ", originSimulationTime = " << originSimulationTime << ", originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << ", getOscillatorCompensation() = " << getOscillatorCompensation() << ", oscillator->getComputationOrigin() = " << oscillator->getComputationOrigin() << ", compensationPhaseBaseTicks = " << compensationPhaseBaseTicks << ", numTicksAtOriginLowerBound = " << numTicksAtOriginLowerBound << std::endl;
        checkAllClockEvents();
        clockTimeBeforeOscillatorStateChange = clockTime;
    }
    else if (signal == IOscillator::postOscillatorStateChangedSignal) {
        clocktime_t clockTime = getClockTime();
        EV_DEBUG << "Handling post-oscillator state changed signal" << EV_FIELD(clockTime) << EV_ENDL;
        ASSERTCMP(==, clockTimeBeforeOscillatorStateChange, clockTime);
        if (useFutureEventSet) {
            simtime_t currentSimTime = simTime();
            std::sort(events.begin(), events.end(), cEvent::compareBySchedulingOrder);
            for (auto event : events) {
                if (event->getRelative()) {
                    clocktime_t clockTimeDelay = event->getArrivalClockTime() - clockTime;
                    simtime_t simulationTimeDelay = computeSimTimeFromClockTime(event->getArrivalClockTime()) - currentSimTime;
                    ASSERT(simulationTimeDelay >= 0);
                    cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                    cContextSwitcher contextSwitcher(targetModule);
                    EV_DEBUG << "Rescheduling clock event after" << EV_FIELD(clockTimeDelay) << EV_FIELD(simulationTimeDelay) << EV_FIELD(event) << EV_ENDL;
                    targetModule->rescheduleAfter(simulationTimeDelay, event);
                }
                else {
                    clocktime_t arrivalClockTime = event->getArrivalClockTime();
                    simtime_t arrivalSimTime = computeSimTimeFromClockTime(arrivalClockTime);
                    ASSERT(arrivalSimTime >= currentSimTime);
                    cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                    cContextSwitcher contextSwitcher(targetModule);
                    EV_DEBUG << "Rescheduling clock event at" << EV_FIELD(arrivalClockTime) << EV_FIELD(arrivalSimTime) << EV_FIELD(event) << EV_ENDL;
                    targetModule->rescheduleAt(arrivalSimTime, event);
                }
                checkClockEvent(event);
            }
        }
        emit(timeChangedSignal, CLOCKTIME_AS_SIMTIME(clockTime));
    }
}

} // namespace inet

