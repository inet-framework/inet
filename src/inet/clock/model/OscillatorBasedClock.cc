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

    originSimulationTime = simulationTime;
    originClockTime = clockTime;
    // Do NOT touch lastClockTime here unless this is a user "set time" op.
    lastClockTime = clockTime;

    // ticks since *current* oscillator origin (>=0)
    const int64_t numTicksFromOscillatorOriginToSimulationTime = oscillator->computeTicksForInterval(simulationTime - oscillator->getComputationOrigin());

    // Global index at the new lower bound
    const int64_t newNumTicksAtOriginLowerBound = oscillator->getNumTicksAtOrigin() + numTicksFromOscillatorOriginToSimulationTime;

    // Simtime of that lower bound (tick-aligned)
    const simtime_t newOriginSimulationTimeLowerBound = oscillator->getComputationOrigin() + oscillator->computeIntervalForTicks(numTicksFromOscillatorOriginToSimulationTime);

    // Phase carry using ONLY global indices (no backward mapping)
    const int64_t d = newNumTicksAtOriginLowerBound - numTicksAtOriginLowerBound;
    ASSERTCMP(>=, d, 0);
    compensationPhaseBaseTicks += d;

    // Canonicalize n0 without changing floor(x*n0)
    const auto& c = getOscillatorCompensation();
    compensationPhaseBaseTicks = c.divCeil(c.mulFloor(compensationPhaseBaseTicks));

    // Commit LB and cache its global index
    originSimulationTimeLowerBound = newOriginSimulationTimeLowerBound;
    numTicksAtOriginLowerBound = newNumTicksAtOriginLowerBound;

    CLOCK_COUT << "   : originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << std::endl;

    // Your original consistency checks
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

int64_t OscillatorBasedClock::computeCompensatedTicksFromTicks(int64_t numTicks) const
{
    // floor(x*(Δn + n0)) - floor(x*n0)
    const auto& oscillatorCompensation = getOscillatorCompensation();
    return oscillatorCompensation.mulFloor(numTicks + compensationPhaseBaseTicks) - oscillatorCompensation.mulFloor(compensationPhaseBaseTicks);
}

int64_t OscillatorBasedClock::computeTicksFromCompensatedTicks(int64_t compensatedTicks) const
{
    // min Δn s.t. floor(x*(Δn + n0)) ≥ M + floor(x*n0)
    const auto& oscillatorCompensation = getOscillatorCompensation();
    const int64_t base = oscillatorCompensation.mulFloor(compensationPhaseBaseTicks);
    const int64_t nprime = oscillatorCompensation.divCeil(base + compensatedTicks);
    return nprime - compensationPhaseBaseTicks;
}

clocktime_t OscillatorBasedClock::doComputeClockTimeFromSimTime(simtime_t simulationTime) const
{
    const clocktime_t nominalTickLength = getClockGranularity();

    // Δn(t) = n(t) − n(LB)
    const int64_t n_t_since_origin = oscillator->computeTicksForInterval(simulationTime - oscillator->getComputationOrigin());
    const int64_t n_lb_since_origin = numTicksAtOriginLowerBound - oscillator->getNumTicksAtOrigin();
    const int64_t delta_n_t = n_t_since_origin - n_lb_since_origin;

    // n0 = n(originSimulationTime) − n(LB)
    const int64_t n_origin_since_origin = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    const int64_t delta_n0 = n_origin_since_origin - n_lb_since_origin;

    // clock(t) = clock(origin) + [ C(Δn(t)) − C(Δn0) ] * tick
    const int64_t comp_t  = computeCompensatedTicksFromTicks(delta_n_t);
    const int64_t comp_n0 = computeCompensatedTicksFromTicks(delta_n0);

    return originClockTime + (comp_t - comp_n0) * nominalTickLength;
}

simtime_t OscillatorBasedClock::doComputeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    const clocktime_t nominalTickLength = getClockGranularity();
    const clocktime_t dClock = clockTime - originClockTime;

    // Compensated ticks requested relative to *originSimulationTime*
    int64_t m = dClock.raw() / nominalTickLength.raw();
    const bool exactMultiple = (dClock.raw() % nominalTickLength.raw()) == 0;
    if (!lowerBound && exactMultiple) ++m; // strict upper bound

    // Compute n0 = n(origin) − n(LB) and its compensated version C(n0)
    const int64_t n_origin_since_origin = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    const int64_t n_lb_since_origin     = numTicksAtOriginLowerBound - oscillator->getNumTicksAtOrigin();
    const int64_t delta_n0              = n_origin_since_origin - n_lb_since_origin;
    const int64_t comp_n0               = computeCompensatedTicksFromTicks(delta_n0);

    // Convert to LB-relative compensated ticks, invert to LB-relative Δn, then shift back by n0
    const int64_t m_lb   = m + comp_n0;
    const int64_t dTicks = computeTicksFromCompensatedTicks(m_lb) - delta_n0; // Δn relative to originSimulationTime

    // n(t) = n(origin) + dTicks, then map to simtime
    const int64_t n_origin_rel = n_origin_since_origin; // ticks from current osc origin to originSimulationTime
    const int64_t n_t_rel      = n_origin_rel + dTicks;

    return oscillator->getComputationOrigin() + oscillator->computeIntervalForTicks(n_t_rel);
}

clocktime_t OscillatorBasedClock::computeClockTimeFromSimTime(simtime_t simulationTime) const
{
    ASSERTCMP(>=, simulationTime, simTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    clocktime_t result = doComputeClockTimeFromSimTime(simulationTime);
    ASSERTCMP(==, result.raw(), result.raw() / getClockGranularity().raw() * getClockGranularity().raw());
    ASSERTCMP(>=, result, doComputeClockTimeFromSimTime(simTime()));
    ASSERTCMP(>=, simulationTime, doComputeSimTimeFromClockTime(result, true));
    ASSERTCMP(<, simulationTime, doComputeSimTimeFromClockTime(result, false));
    return result;
}

simtime_t OscillatorBasedClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    ASSERTCMP(>=, clockTime, getClockTime());
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    ASSERTCMP(==, clockTime.raw(), clockTime.raw() / getClockGranularity().raw() * getClockGranularity().raw());
    simtime_t result = doComputeSimTimeFromClockTime(clockTime, lowerBound);
    ASSERTCMP(>=, result, originSimulationTimeLowerBound);
    if (lowerBound)
        ASSERTCMP(==, clockTime, doComputeClockTimeFromSimTime(result))
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
        checkAllClockEvents();
        setOrigin(simTime(), clockTime);
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

