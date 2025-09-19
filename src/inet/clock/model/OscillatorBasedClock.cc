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
        ASSERTCMP(==, currentClockTime.raw() % getClockGranularity().raw(), 0);
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
        checkAllClockEvents();
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
static inline uint64_t frac_advance(uint64_t p_q63, int64_t e_q63, int64_t m)
{
    const U128 MOD = U128(1) << 63;
    S128 sum_q63 = (S128)p_q63 - (S128)e_q63 * (S128)m; // Q1.63
    return (uint64_t)((U128)sum_q63 % MOD);
}

// floor_q63(p - e * m)
static inline int64_t A_rel(int64_t m, uint64_t p_q63, int64_t e_q63)
{
    const S128 q = (S128)p_q63 - (S128)e_q63 * (S128)m;
    const S128 qd = floor_div(q, S128_ONE_Q63);
    return (int64_t)qd;
}

int64_t OscillatorBasedClock::A(int64_t n) const
{
    const uint64_t p = oscillatorCompensationAccumulator;
    const S64 e_q63 = getOscillatorCompensation().raw(); // e = 1 - x (Q1.63)
    const int64_t n0 = oscillator->computeTicksForInterval(originSimulationTime - oscillator->getComputationOrigin());
    const int64_t m  = n - n0;
    const S128 q = (S128)p - (S128)e_q63 * (S128)m; // Q1.63
    const S128 qd = floor_div(q, S128_ONE_Q63);
    return (int64_t)qd;
}

void OscillatorBasedClock::checkState() const
{
    ASSERTCMP(>=, originSimulationTime, oscillator->getComputationOrigin());
    ASSERTCMP(<=, originSimulationTimeLowerBound, originSimulationTime);
    ASSERTCMP(<=, originSimulationTimeLowerBound, simTime());
    ASSERTCMP(<=, oscillator->getComputationOrigin(), originSimulationTimeLowerBound);
    ASSERTCMP(==, originSimulationTimeLowerBound, computeSimTimeFromClockTime(originClockTime, true));
    ASSERTCMP(<=, originSimulationTime, simTime());
    ASSERTCMP(<=, oscillator->getComputationOrigin(), originSimulationTime);
    ASSERTCMP(<=, originSimulationTimeLowerBound, originSimulationTime);
    ASSERTCMP(<=, originSimulationTime, computeSimTimeFromClockTime(originClockTime, false));
    ASSERTCMP(==, originClockTime, computeClockTimeFromSimTime(originSimulationTime));
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
    const uint64_t p = oscillatorCompensationAccumulator;
    // implement formula
    const int64_t n = oscillator->computeTicksForInterval(s - oos);
    const int64_t n0 = oscillator->computeTicksForInterval(cos - oos);
    const int64_t m = n - n0;
    const int64_t e_q63 = getOscillatorCompensation().raw();
    const int64_t dF_rel = m + A_rel(m, p, e_q63);
    const clocktime_t coc_new = ClockTime::fromRaw((S128)coc_old.raw() + (S128)dF_rel * (S128)l);
    const uint64_t p_new = frac_advance(p, e_q63, m);
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
    ASSERTCMP(>=, simulationTime, originSimulationTime);
    ASSERTCMP(>=, simulationTime, oscillator->getComputationOrigin());
    EV_DEBUG << "Setting clock origin" << EV_FIELD(simulationTime) << EV_FIELD(clockTime) << EV_ENDL;
    // get state
    const simtime_t& s = simTime();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const uint64_t p = oscillatorCompensationAccumulator;
    // implement formula
    const int64_t n = oscillator->computeTicksForInterval(s - oos);
    const int64_t n0 = oscillator->computeTicksForInterval(cos - oos);
    const int64_t m = n - n0;
    const int64_t e_q63 = getOscillatorCompensation().raw();
    const uint64_t p_new = frac_advance(p, e_q63, m);
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

// TODO: add lowerBound, see https://chatgpt.com/c/68b1993a-5b00-8321-99f8-bfa009722028
clocktime_t OscillatorBasedClock::doComputeClockTimeFromSimTime(simtime_t simulationTime, bool lowerBound) const
{
    DEBUG_ENTER(true, simulationTime);
   // get state
    const clocktime_raw_t l = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const clocktime_t& coc = originClockTime;
    const simtime_t& s = simulationTime;
    // implement formula
    const S64 n  = oscillator->computeTicksForInterval(s - oos);
    const S64 n0 = oscillator->computeTicksForInterval(cos - oos);
    S64 m  = n - n0;
    // exact boundary detection
    const bool atBoundary = (s == (oos + oscillator->computeIntervalForTicks(n)));

    // choose effective m based on bound
    S64 m_eff = m;
    if (!lowerBound && atBoundary && m_eff > 0)
        --m_eff;                               // open interval end maps to previous step

    // A_rel(m) = floor_q63(p - e*m)
    const S64  e_q63 = getOscillatorCompensation().raw(); // Q1.63 (e = 1 - x)
    const uint64_t p_q63 = oscillatorCompensationAccumulator;
    auto A_rel = [&](S64 mm)->S64 {
        const S128 q  = (S128)p_q63 - (S128)e_q63 * (S128)mm;
        const S128 qd = q / S128_ONE_Q63;
        const S128 r  = q % S128_ONE_Q63;
        return (S64)(qd - ((r != 0 && q < 0) ? 1 : 0));   // floor fix
    };

    if (m_eff < 0) m_eff = 0;                   // clamp

    const S64 dF = m_eff + A_rel(m_eff);
    const S128 c_raw = (S128)coc.raw() + (S128)dF * (S128)l;
    DEBUG_OUT << DEBUG_FIELD(n) << DEBUG_FIELD(n0) << DEBUG_FIELD(dF) << std::endl;
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
    const uint64_t p = oscillatorCompensationAccumulator;
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
    ASSERTCMP(>=, simulationTime, simTime());
    clocktime_t result = doComputeClockTimeFromSimTime(simulationTime, lowerBound);
    ASSERTCMP(==, result.raw() % getClockGranularity().raw(), 0);
    ASSERTCMP(>=, result, doComputeClockTimeFromSimTime(simTime(), true));
    ASSERTCMP(>=, simulationTime, doComputeSimTimeFromClockTime(result, true));
    ASSERTCMP(<=, simulationTime, doComputeSimTimeFromClockTime(result, false));
    return result;
}

simtime_t OscillatorBasedClock::computeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    ASSERTCMP(>=, clockTime, getClockTime());
    ASSERTCMP(==, clockTime.raw() % getClockGranularity().raw(), 0);
    simtime_t result = doComputeSimTimeFromClockTime(clockTime, lowerBound);
    ASSERTCMP(>=, result, originSimulationTimeLowerBound); // TODO: more restrictive assert for now?
//    if (lowerBound)
//        ASSERTCMP(==, clockTime, doComputeClockTimeFromSimTime(result)) // TODO: what happens if due to oscillator compensation a specific clock time value was skipped
//        ASSERTCMP(<=, clockTime, doComputeClockTimeFromSimTime(result)) // maybe this?
//    else
//        // uses inequality because oscillator compensation
//        ASSERTCMP(<=, clockTime + getClockGranularity(), doComputeClockTimeFromSimTime(result));
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
        moveOrigin();
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

