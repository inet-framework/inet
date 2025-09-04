//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSCILLATORBASEDCLOCK_H
#define __INET_OSCILLATORBASEDCLOCK_H

#include "inet/clock/base/ClockBase.h"
#include "inet/clock/contract/IOscillator.h"
#include "inet/common/SimTimeScale.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

static bool compareClockEvents(const ClockEvent *e1, const ClockEvent *e2) {
    return e2->getArrivalClockTime() < e1->getArrivalClockTime() ? true :
           e2->getArrivalClockTime() > e1->getArrivalClockTime() ? false :
           e2->getSchedulingPriority() == e1->getSchedulingPriority() ? e2->getInsertionOrder() < e1->getInsertionOrder() :
           e2->getSchedulingPriority() < e1->getSchedulingPriority() ? true :
           e2->getSchedulingPriority() > e1->getSchedulingPriority() ? false :
           e2->getInsertionOrder() < e1->getInsertionOrder();
}

/**
 * @brief A clock model that is counting the ticks of an oscillator.
 *
 * The following properties always hold for all oscillator based clocks:
 *
 * 1. The clock origin simulation time is always greater than or equal to the computation origin of the oscillator.
 *
 * 2. The clock origin simulation time is always less than or equal to the current simulation time.
 *
 * Mathematical definitions:
 *  - b: bound; true means the lower/inclusive bound; false means the upper/exclusive bound
 *  - l: nominal tick length [s], l > 0
 *  - oos: oscillator origin simulation time [s], oos >= 0
 *  - cos: clock origin simulation time [s], cos >= 0, cos >= oos
 *  - coc: clock origin clock time [s], coc >= 0
 *  - x: oscillator compensation factor ~1, x > 0
 *  - p: oscillator compensation fractional accumulator [-], p >= 0
 *  - s: simulation time [s], s >= 0, s >= oos
 *  - c: clock time [s], c >= 0, c >= coc
 *  - n = numTicks(i): number of ticks from oscillator origin for interval, i >= 0, n >= 0
 *  - i = interval(n): simulation time interval from oscillator origin, i >= 0, n >= 0
 *  - A(n) = floor(p + (x - 1) * n): compensation events up to tick index n (relative to oscillator origin, applying past carry-in p)
 *  - F(n) = n + A(n): effective “output ticks” accumulated up to n
 *  - p(n) = fractional(p + (x - 1) * n) in [0, 1): residual fractional compensation after n ticks
 *
 * The functions numTicks() and interval() are mutually consistent inverses.
 */
class INET_API OscillatorBasedClock : public ClockBase, public cListener
{
  protected:
    // parameters
    bool useFutureEventSet = false;
    IOscillator *oscillator = nullptr;
    simtime_raw_t (*roundingFunction)(simtime_raw_t, simtime_raw_t) = nullptr;

    // state variables
    /**
     * The lower bound of the simulation time where the clock time equals with origin clock time (exactly at an oscillator tick).
     */
    simtime_t originSimulationTimeLowerBound;
    /**
     * The simulation time from which the clock computes the mapping between future simulation times and clock times.
     */
    simtime_t originSimulationTime;
    /**
     * The clock time from which the clock computes the mapping between future simulation times and clock times.
     */
    clocktime_t originClockTime;
    /**
     * Fractional accumulator (Q0.63) for the oscillator compensation at the clock origin.
     * Stored modulo 2^63 in [0, 2^63); represents p = frac(p + (x - 1) * n) scaled by 2^63.
     */
    uint64_t oscillatorCompensationAccumulator = 0;
    /**
     * The clock time accumulated from the oscillator compensation.
     */
    clocktime_t clockTimeCompensation;

    uint64_t insertionCount = 0;
    uint64_t lastNumTicks = 0;
    clocktime_t clockTimeBeforeOscillatorStateChange = -1; // used for assertion

    // events and timers
    std::vector<ClockEvent *> events;
    cMessage *executeEventsTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event) override;
    virtual void scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event) override;
    virtual ClockEvent *cancelTargetModuleClockEvent(ClockEvent *event) override;

    virtual void checkState() const;

    virtual void checkAllScheduledClockEvents() const;

    /**
     * Moves the clock origin to the current simulation time such that the simulation time <-> clock time mapping is unaffected.
     *
     * Mathematical formula: moveOrigin()
     *   s  = current simulation time
     *   n  = numTicks(s − oos)
     *   n0 = numTicks(cos − oos)
     *   m  = n − n0
     *   cos = s
     *   coc = coc + (F(n) − F(n0)) * l
     *   p   = fractional(p + (x − 1) * m)
     */
    virtual void moveOrigin();

    /**
     * Updates the clock origin to the current simulation time and the given clock time.
     *
     * Mathematical formula: setOrigin(c)
     *   s  = current simulation time
     *   n  = numTicks(s − oos)
     *   n0 = numTicks(cos − oos)
     *   m  = n − n0
     *   cos = s
     *   coc = c
     *   p   = fractional(p + (x − 1) * m)
     */
    virtual void setOrigin(clocktime_t clockTime);

    /**
     * A(n) = floor(p + (x - 1) * (n - n0)) = floor(p - e * (n - n0))
     */
    int64_t A(int64_t n) const;
    int64_t F(int64_t n) const { return n + A(n); }

    /**
     * Mathematical formula: c = doComputeClockTimeFromSimTime(s, b):
     *   n  = numTicks(s - oos)
     *   n0 = numTicks(cos - oos)
     *   atBoundary = (s == oos + interval(n))
     *   m  = n - n0
     *   m  = b ? m : (atBoundary && m > 0 ? m - 1 : m)
     *   c  = coc + (F(n0 + m) - F(n0)) * l
     */
    clocktime_t doComputeClockTimeFromSimTime(simtime_t t, bool lowerBound) const;

    /**
     * Mathematical formula: s = doComputeSimTimeFromClockTime(c, b):
     *   n0 = numTicks(cos - oos)
     *   k  = floor((c - coc) / l) + (b ? 0 : 1)
     *   m1 = min{ m ≥ 0 : m + floor(p + (x − 1) * m) ≥ k }
     *   Closed-form inverse of m + floor(p + (x − 1) * m) (x > 0):
     *     m1 = ceil((k − p) / x)
     *   n1 = n0 + max(0, m1)
     *   s  = oos + interval(n1)
     */
    simtime_t doComputeSimTimeFromClockTime(clocktime_t t, bool lowerBound) const;

    clocktime_t getClockGranularity() const { return SIMTIME_AS_CLOCKTIME(oscillator->getNominalTickLength()); }

  public:
    virtual ~OscillatorBasedClock();

    virtual clocktime_t getClockTime() const override;

    virtual const IOscillator *getOscillator() const { return oscillator; }
    virtual SimTimeScale getOscillatorCompensation() const { return SimTimeScale(); }

    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t, bool lowerBound = true) const override;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t, bool lowerBound = true) const override;

    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;
    virtual bool isScheduledClockEvent(ClockEvent *event) const override;

    virtual std::string resolveDirective(char directive) const override;

    virtual void receiveSignal(cComponent *source, int signal, uintval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

