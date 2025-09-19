//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/OscillatorBasedClock.h"

#include "inet/common/IPrintableObject.h"

#include <algorithm>

namespace inet {

using S64  = int64_t;
using U64  = uint64_t;
using S128 = __int128_t;
using U128 = __uint128_t;

const S128 S128_ONE_Q63 = (S128)1 << 63;
const U128 U128_ONE_Q63 = (U128)1 << 63;

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
        DEBUG_CMP(currentClockTime.raw() % getClockGranularity().raw(), ==, 0);
        return currentClockTime;
    }
    else {
        // NOTE: IClock interface 1. invariant
        DEBUG_CMP(originClockTime, >=, lastClockTime);
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
        WATCH(oscillatorCompensationAccumulator);
        WATCH_PTRVECTOR(events);
    }
    else if (stage == INITSTAGE_CLOCK) {
        setOrigin(par("initialClockTime"));
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
        checkAllScheduledClockEvents();
    }
    else
        ClockBase::handleMessage(message);
}

static inline S128 floor_div(S128 a, S128 b)
{
    S128 q = a / b, r = a % b;
    if (r != 0 && ((a < 0) != (b < 0))) --q;
    return q;
}

// frac in Q0.63: keep modulo 2^63
static inline U64 frac_advance(U64 p_q63, S64 e_q63, S64 m)
{
    const U128 MOD = U128(1) << 63;
    S128 sum_q63 = (S128)p_q63 - (S128)e_q63 * (S128)m; // Q1.63
    return (U64)((U128)sum_q63 % MOD);
}

// floor_q63(p - e * m)
static inline S64 A_rel(S64 m, U64 p_q63, S64 e_q63)
{
    const S128 q = (S128)p_q63 - (S128)e_q63 * (S128)m;
    const S128 qd = floor_div(q, S128_ONE_Q63);
    return (S64)qd;
}

S64 OscillatorBasedClock::A(S64 n) const
{
    const U64 p = oscillatorCompensationAccumulator;
    const S64 e_q63 = getOscillatorCompensation().raw(); // e = 1 - x (Q1.63)
    const S64 n0 = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    const S64 m  = n - n0;
    const S128 q = (S128)p - (S128)e_q63 * (S128)m; // Q1.63
    const S128 qd = floor_div(q, S128_ONE_Q63);
    return (S64)qd;
}

void OscillatorBasedClock::checkState() const
{
    // originSimulationTime constraints
    DEBUG_CMP(oscillator->getComputationOrigin(), <=, originSimulationTime);
    DEBUG_CMP(originSimulationTime, <=, simTime());
    // originSimulationTimeLowerBound constraints
    DEBUG_CMP(oscillator->getComputationOrigin(), <=, originSimulationTimeLowerBound);
    DEBUG_CMP(originSimulationTimeLowerBound, <=, originSimulationTime);
    DEBUG_CMP(originSimulationTimeLowerBound, ==, computeSimTimeFromClockTime(originClockTime, true));
    // simulation time <-> clock time mapping constraints for origin
    DEBUG_CMP(computeSimTimeFromClockTime(originClockTime, true), <=, originSimulationTime);
    DEBUG_CMP(originSimulationTime, <=, computeSimTimeFromClockTime(originClockTime, false));
    DEBUG_CMP(originClockTime, ==, computeClockTimeFromSimTime(originSimulationTime));
}

void OscillatorBasedClock::moveOrigin()
{
    // debug output
    DEBUG_ENTER(true);
    // get state
    const clocktime_raw_t l = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const clocktime_t& coc_old = originClockTime;
    const simtime_t& s = simTime();
    const U64 p = oscillatorCompensationAccumulator;
    // implement formula
    const S64 n = oscillator->computeTicksForInterval(s - oos);
    const S64 n0 = oscillator->computeTicksForInterval(cos - oos);
    const S64 m = n - n0;
    const S64 e_q63 = getOscillatorCompensation().raw();
    const S64 dF_rel = m + A_rel(m, p, e_q63);
    const clocktime_t coc_new = ClockTime::fromRaw((S128)coc_old.raw() + (S128)dF_rel * (S128)l);
    const U64 p_new = frac_advance(p, e_q63, m);
    // update state
    originSimulationTimeLowerBound = oos + oscillator->computeIntervalForTicks(n);
    originSimulationTime = s;
    originClockTime = coc_new;
    oscillatorCompensationAccumulator = p_new;
    // debug output
    DEBUG_OUT << DEBUG_FIELD(originSimulationTimeLowerBound) << DEBUG_FIELD(originSimulationTime) << DEBUG_FIELD(originClockTime) << DEBUG_FIELD(oscillatorCompensationAccumulator) << std::endl;
    DEBUG_LEAVE();
    checkState();
}

void OscillatorBasedClock::setOrigin(clocktime_t clockTime)
{
    // debug output
    DEBUG_ENTER(true, clockTime);
    // checks
    simtime_t simulationTime = simTime();
    DEBUG_CMP(simulationTime, >=, originSimulationTime);
    DEBUG_CMP(simulationTime, >=, oscillator->getComputationOrigin());
    EV_DEBUG << "Setting clock origin" << EV_FIELD(simulationTime) << EV_FIELD(clockTime) << EV_ENDL;
    // get state
    const simtime_t& s = simTime();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const U64 p = oscillatorCompensationAccumulator;
    // implement formula
    const S64 n = oscillator->computeTicksForInterval(s - oos);
    const S64 n0 = oscillator->computeTicksForInterval(cos - oos);
    const S64 m = n - n0;
    const S64 e_q63 = getOscillatorCompensation().raw();
    const U64 p_new = frac_advance(p, e_q63, m);
    // update state
    originSimulationTimeLowerBound = oscillator->getComputationOrigin() + oscillator->computeIntervalForTicks(n);
    originSimulationTime = s;
    originClockTime = clockTime;
    oscillatorCompensationAccumulator = p_new;
    lastClockTime = clockTime;
    // debug output
    DEBUG_OUT << DEBUG_FIELD(originSimulationTimeLowerBound) << DEBUG_FIELD(originSimulationTime) << DEBUG_FIELD(originClockTime) << DEBUG_FIELD(oscillatorCompensationAccumulator) << DEBUG_FIELD(lastClockTime) << std::endl;
    DEBUG_LEAVE();
    checkState();
}

clocktime_t OscillatorBasedClock::doComputeClockTimeFromSimTime(simtime_t simulationTime, bool lowerBound) const
{
    DEBUG_ENTER(true, simulationTime);
   // get state
    const clocktime_raw_t l = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const clocktime_t& coc = originClockTime;
    const simtime_t& s = simulationTime;
    const U64 p = oscillatorCompensationAccumulator;
    const S64  e_q63 = getOscillatorCompensation().raw();
    // implement formula
    const S64 n  = oscillator->computeTicksForInterval(s - oos);
    const S64 n0 = oscillator->computeTicksForInterval(cos - oos);
    const bool atBoundary = (s == (oos + oscillator->computeIntervalForTicks(n)));
    S64 m  = n - n0;
    if (!lowerBound && atBoundary && m > 0) --m;
    if (m < 0) m = 0;
    const S64 dF = m + A_rel(m, p, e_q63);
    const S128 c_raw = (S128)coc.raw() + (S128)dF * (S128)l;
    DEBUG_OUT << DEBUG_FIELD(n) << DEBUG_FIELD(n0) << DEBUG_FIELD(m) << DEBUG_FIELD(dF) << std::endl;
    clocktime_t result = ClockTime::fromRaw(c_raw);
    DEBUG_LEAVE(result);
    return result;
}

simtime_t OscillatorBasedClock::doComputeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    DEBUG_ENTER(true, clockTime, lowerBound);
    // get state
    const bool b = lowerBound;
    const clocktime_raw_t l = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const clocktime_t& coc = originClockTime;
    const clocktime_t& c = clockTime;
    const U64 p = oscillatorCompensationAccumulator;
    const S64 e_q63 = getOscillatorCompensation().raw(); // Q1.63
    const U128 p_q63 = (U128)p % U128_ONE_Q63; // Q0.63
    // implement formula
    const S64 n0 = oscillator->computeTicksForInterval(cos - oos);
    const S128 k = floor_div((S128)(c - coc).raw(), (S128)l) + (b ? 0 : 1); // integer
    // solve m = ceil((k - p) / x), x = 1 - e.
    auto ceil_div_q63_by_x = [&] (S128 N_q63)->S64 {
        U128 D = U128(U128_ONE_Q63) - U128((S128)e_q63); // D = x * 2^63 in (0, 2^64]
        if (N_q63 >= 0) {
            U128 N = (U128)N_q63;
            return (S64)((N + (D - 1)) / D);
        }
        else {
            U128 A = (U128)(-N_q63);
            return -(S64)(A / D); // ceil(neg/pos) = -floor(|neg|/pos)
        }
    };
    const S128 N_q63 = (k << 63) - (S128)p_q63; // (k - p) in Q1.63
    S64 m1 = ceil_div_q63_by_x(N_q63);
    if (m1 < 0) m1 = 0;
    const S64 n1 = n0 + m1;
    DEBUG_OUT << DEBUG_FIELD(n0) << DEBUG_FIELD(k) << DEBUG_FIELD(m1) << DEBUG_FIELD(n1) << std::endl;
    const simtime_t s = oos + oscillator->computeIntervalForTicks(n1);
    DEBUG_LEAVE(s);
    return s;
}

clocktime_t OscillatorBasedClock::computeClockTimeFromSimTime(simtime_t simulationTime, bool lowerBound) const
{
    DEBUG_CMP(simulationTime, >=, simTime());
    clocktime_t result = doComputeClockTimeFromSimTime(simulationTime, lowerBound);
    DEBUG_CMP(result.raw() % getClockGranularity().raw(), ==, 0);
    DEBUG_CMP(result, >=, doComputeClockTimeFromSimTime(simTime(), true));
    DEBUG_CMP(simulationTime, >=, doComputeSimTimeFromClockTime(result, true));
    DEBUG_CMP(simulationTime, <=, doComputeSimTimeFromClockTime(result, false));
    return result;
}

simtime_t OscillatorBasedClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    DEBUG_CMP(clockTime, >=, getClockTime());
    DEBUG_CMP(clockTime.raw() % getClockGranularity().raw(), ==, 0);
    simtime_t result = doComputeSimTimeFromClockTime(clockTime, lowerBound);
    DEBUG_CMP(result, >=, originSimulationTimeLowerBound); // TODO: more restrictive assert for now?
//    if (lowerBound)
//        DEBUG_CMP(clockTime, ==, doComputeClockTimeFromSimTime(result)) // TODO: what happens if due to oscillator compensation a specific clock time value was skipped
//        DEBUG_CMP(clockTime, <=, doComputeClockTimeFromSimTime(result)) // maybe this?
//    else
//        // uses inequality because oscillator compensation
//        DEBUG_CMP(clockTime + getClockGranularity(), <=, doComputeClockTimeFromSimTime(result));
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
    DEBUG_CMP(time, >=, getClockTime());
    simtime_raw_t roundedTime = roundingFunction(time.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAt(ClockTime().setRaw(roundedTime), event);
    events.push_back(event);
    if (!useFutureEventSet)
        event->setInsertionOrder(insertionCount++);
    checkScheduledClockEvent(event);
}

void OscillatorBasedClock::scheduleClockEventAfter(clocktime_t delay, ClockEvent *event)
{
    DEBUG_CMP(delay, >=, 0);
    simtime_raw_t roundedDelay = roundingFunction(delay.raw(), oscillator->getNominalTickLength().raw());
    ClockBase::scheduleClockEventAfter(ClockTime().setRaw(roundedDelay), event);
    events.push_back(event);
    if (!useFutureEventSet)
        event->setInsertionOrder(insertionCount++);
    checkScheduledClockEvent(event);
}

ClockEvent *OscillatorBasedClock::cancelClockEvent(ClockEvent *event)
{
    checkScheduledClockEvent(event);
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
            S64 numTicks = clockTimeCompensation.raw() / oscillator->getNominalTickLength().raw();
            clocktime_t clockTimeDelta = SIMTIME_AS_CLOCKTIME(numTicks * oscillator->getNominalTickLength());
            clockTimeCompensation -= clockTimeDelta;
            setOrigin(originClockTime + clockTimeDelta);
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
        checkAllScheduledClockEvents();
    }
}

void OscillatorBasedClock::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == IOscillator::preOscillatorStateChangedSignal) {
        clocktime_t clockTime = getClockTime();
        EV_DEBUG << "Handling pre-oscillator state changed signal" << EV_FIELD(clockTime) << EV_ENDL;
        checkAllScheduledClockEvents();
        moveOrigin();
        checkAllScheduledClockEvents();
        clockTimeBeforeOscillatorStateChange = clockTime;
    }
    else if (signal == IOscillator::postOscillatorStateChangedSignal) {
        clocktime_t clockTime = getClockTime();
        EV_DEBUG << "Handling post-oscillator state changed signal" << EV_FIELD(clockTime) << EV_ENDL;
        DEBUG_CMP(clockTimeBeforeOscillatorStateChange, ==, clockTime);
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
                checkScheduledClockEvent(event);
            }
        }
        emit(timeChangedSignal, CLOCKTIME_AS_SIMTIME(clockTime));
    }
}

} // namespace inet

