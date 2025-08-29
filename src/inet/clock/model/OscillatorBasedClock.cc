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

// A(n) = floor(p + (x - 1) * n) = floor(p - e * n), with
// p in Q0.63 (uint64_t, modulo 2^63) and e_q63 = 1 - x in Q1.63 (int64_t).
int64_t OscillatorBasedClock::A(int64_t n) const {
    using S128 = __int128_t;
    const S128 ONE_Q63 = (S128)1 << 63;

    const int64_t e_q63 = getOscillatorCompensation().raw();   // Q1.63 (e = 1 - x)
    S128 q = (S128)p - (S128)e_q63 * (S128)n;                  // Q1.63

    // floor(q / 2^63) with correct handling for negative q
    S128 qd = q / ONE_Q63;
    S128 r  = q % ONE_Q63;
    if (r != 0 && q < 0) --qd;
    return (int64_t)qd;
}

int64_t OscillatorBasedClock::F(int64_t n) const {
    return n + A(n);
}

void OscillatorBasedClock::moveOrigin()
{
    using S128 = __int128_t;

    const clocktime_raw_t l  = getClockGranularity().raw();
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& cos = originSimulationTime;
    const simtime_t& s   = simTime();
    const clocktime_t& coc = originClockTime;

    auto numTicks = [=] (auto interval) { return oscillator->computeTicksForInterval(interval); };

    const int64_t n  = numTicks(s   - oos);
    const int64_t n0 = numTicks(cos - oos);
    const int64_t dF = F(n) - F(n0);
    const S128 coc_raw = (S128)coc.raw() + (S128)dF * (S128)l;

    setOrigin(ClockTime::fromRaw(coc_raw));
}

void OscillatorBasedClock::setOrigin(clocktime_t clockTime)
{
    using S128 = __int128_t;
    using U128 = __uint128_t;

    simtime_t simulationTime = simTime();
    EV_DEBUG << "Setting clock origin" << EV_FIELD(simulationTime) << EV_FIELD(clockTime) << EV_ENDL;
    ClockCoutIndent indent;
    CLOCK_COUT << "-> setOrigin(" << clockTime << ")\n";

    ASSERTCMP(>=, simulationTime, originSimulationTime);
    ASSERTCMP(>=, simulationTime, oscillator->getComputationOrigin());

    const int64_t numTicksFromOscillatorOriginToSimulationTime = oscillator->computeTicksForInterval(simulationTime - oscillator->getComputationOrigin());
    const simtime_t newOriginSimulationTimeLowerBound = oscillator->getComputationOrigin() + oscillator->computeIntervalForTicks(numTicksFromOscillatorOriginToSimulationTime);

    // mathematical definitions
    const simtime_t& oos = oscillator->getComputationOrigin();
    const simtime_t& s = simTime();
    const clocktime_t& c = clockTime;
    auto numTicks = [=] (auto interval) { return oscillator->computeTicksForInterval(interval); };
    const int64_t n = numTicks(s - oos);
    const simtime_t cos = s;
    const clocktime_t coc = c;

    // Update p (Q0.63, modulo 2^63): p <- frac(p + (x - 1) * n) = frac(p - e * n)
    const int64_t e_q63 = getOscillatorCompensation().raw();   // Q1.63 (e = 1 - x)
    const U128 MOD = (U128)1 << 63;
    S128 sum_q63 = (S128)p - (S128)e_q63 * (S128)n;            // Q1.63
    p = (uint64_t)((U128)sum_q63 % MOD);                       // keep residue in [0,2^63)

    // commit state
    originSimulationTimeLowerBound = newOriginSimulationTimeLowerBound;
    originSimulationTime = cos;
    originClockTime = coc;
    lastClockTime = clockTime;

    CLOCK_COUT << "   : originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << std::endl;

    // consistency checks
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
    using S64  = int64_t;
    using S128 = __int128_t;

    const simtime_t&   oos = oscillator->getComputationOrigin();
    const simtime_t&   cos = originSimulationTime;
    const clocktime_t& coc = originClockTime;
    const simtime_t&   s   = simulationTime;
    const clocktime_raw_t l = getClockGranularity().raw();
    auto numTicks = [&](simtime_t i) -> S64 { return oscillator->computeTicksForInterval(i); };

    const S64 n  = numTicks(s   - oos);
    const S64 n0 = numTicks(cos - oos);
    const S64 dF = F(n) - F(n0);
    const S128 c_raw = (S128)coc.raw() + (S128)dF * (S128)l;
    return ClockTime::fromRaw(c_raw);
}

simtime_t OscillatorBasedClock::doComputeSimTimeFromClockTime(clocktime_t clockTime, bool lowerBound) const
{
    using S64  = int64_t;
    using S128 = __int128_t;
    using U128 = __uint128_t;

    const bool b = lowerBound;

    const simtime_t&  oos = oscillator->getComputationOrigin();
    const simtime_t&  cos = originSimulationTime;
    const clocktime_t& coc = originClockTime;
    const clocktime_t& c   = clockTime;
    const clocktime_raw_t l = getClockGranularity().raw();
    auto numTicks   = [&](simtime_t i) -> S64 { return oscillator->computeTicksForInterval(i); };
    auto intervalOf = [&](S64 n) -> simtime_t { return oscillator->computeIntervalForTicks(n); };

    // signed floor division for 128/128 (b>0)
    auto floor_div = [] (S128 a, S128 b) -> S128 {
        ASSERT(b > 0);
        S128 q = a / b, r = a % b;
        if (r != 0 && ((a < 0) != (b < 0))) --q;
        return q;
    };

    // compensation state
    const S64 e_q63 = getOscillatorCompensation().raw(); // Q1.63 (e = 1-x). Here e_q63=0.
    const U128 ONE_Q63 = U128(1) << 63;
    U128 p_q63 = U128(p) % ONE_Q63;                      // Q0.63 residue (here 0)

    // A(n) = floor( p - e*n )
    auto A_of = [&] (S64 n) -> S64 {
        S128 q = S128(p_q63) - S128(e_q63) * S128(n);
        S128 qd = q / S128(ONE_Q63);
        S128 r  = q % S128(ONE_Q63);
        if (r != 0 && q < 0) --qd;
        return (S64)qd;
    };

    const S64 n0  = numTicks(cos - oos);
    const S64 F_n0 = n0 + A_of(n0);

    // *** use full-width raw for numerator & denominator ***
    const S128 dc = (S128)(c - coc).raw();
    const S128 k  = floor_div(dc, (S128)l) + (b ? 0 : 1);

    const S128 T = (S128)F_n0 + k;

    // Build N_q63 = (T - p) / x in Q1.63
    S128 N_q63 = (T << 63) - (S128)p_q63;
//    if (!b) N_q63 += S128(ONE_Q63);

    // n1 = ceil( N_q63 / x ), x = 1 - e, denom D = 2^63 - e_q63
    auto ceil_div_q63_by_x = [&] (S128 N_q63_local) -> S64 {
        U128 D = U128(ONE_Q63) - U128((S128)e_q63);     // D in (0, 2^64]
        if (N_q63_local >= 0) {
            U128 N = (U128)N_q63_local;
            return (S64)((N + (D - 1)) / D);
        } else {
            U128 A = (U128)(-N_q63_local);
            return -(S64)(A / D);                       // ceil(neg/pos) = -floor(|neg|/pos)
        }
    };

    S64 n1 = ceil_div_q63_by_x(N_q63);
    if (n1 < 0) n1 = 0;

    return oos + intervalOf(n1);
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
//        std::cout << "Before setOrigin() call" << ", originClockTime = " << originClockTime << ", originSimulationTime = " << originSimulationTime << ", originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << ", getOscillatorCompensation() = " << getOscillatorCompensation() << ", oscillator->getComputationOrigin() = " << oscillator->getComputationOrigin() << ", compensationPhaseBaseTicks = " << compensationPhaseBaseTicks << std::endl;
        checkAllClockEvents();
        moveOrigin();
//        std::cout << "After  setOrigin() call" << ", originClockTime = " << originClockTime << ", originSimulationTime = " << originSimulationTime << ", originSimulationTimeLowerBound = " << originSimulationTimeLowerBound << ", getOscillatorCompensation() = " << getOscillatorCompensation() << ", oscillator->getComputationOrigin() = " << oscillator->getComputationOrigin() << ", compensationPhaseBaseTicks = " << compensationPhaseBaseTicks << std::endl;
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

