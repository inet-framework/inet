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

/**
 * Strict total order for ClockEvent scheduling.
 *
 * Order by increasing arrival clock time; on ties, by decreasing scheduling
 * priority (higher first); on ties, by insertion order (FIFO).
 */
static bool compareClockEvents(const ClockEvent *e1, const ClockEvent *e2) {
    return e2->getArrivalClockTime() < e1->getArrivalClockTime() ? true :
           e2->getArrivalClockTime() > e1->getArrivalClockTime() ? false :
           e2->getSchedulingPriority() == e1->getSchedulingPriority() ? e2->getInsertionOrder() < e1->getInsertionOrder() :
           e2->getSchedulingPriority() < e1->getSchedulingPriority() ? true :
           e2->getSchedulingPriority() > e1->getSchedulingPriority() ? false :
           e2->getInsertionOrder() < e1->getInsertionOrder();
}

/**
 * @brief Clock model driven by an underlying IOscillator tick sequence.
 *
 * Overview
 * --------
 * This clock maps between simulation time and clock time by counting oscillator
 * ticks since an oscillator computation origin and applying a (near-1) per-tick
 * compensation. It supports both absolute (“at”) and relative (“after”) clock
 * scheduling (see ~IClock) without generating per-tick events.
 *
 * Invariants
 * ----------
 * 1) The clock origin simulation time (cos) is always >= the oscillator’s
 *    computation origin (oos).
 * 2) The clock origin simulation time (cos) is always <= the current simulation
 *    time (now).
 *
 * Symbols (all times in seconds unless noted)
 * -------------------------------------------
 *  - b    : bound selector; true = lower/inclusive, false = upper/exclusive
 *  - l    : nominal tick length, l > 0
 *  - oos  : oscillator origin simulation time, oos >= 0
 *  - cos  : clock origin simulation time, cos >= 0, cos >= oos
 *  - coc  : clock origin clock time, coc >= 0
 *  - x    : oscillator compensation factor, near 1, x > 0
 *  - p    : fractional accumulator in [0,1) for compensation carry-over
 *  - s    : simulation time argument, s >= oos
 *  - c    : clock time argument,   c >= coc
 *  - n    : numTicks(i): # of oscillator ticks in interval i >= 0
 *  - i    : interval(n): minimal simulation time to include n ticks (from oos)
 *
 * Compensation step function
 * --------------------------
 * Let e := 1 − x and define, for tick index n (from oscillator origin):
 *  - A(n) = floor( p + (x − 1) * n ) = floor( p − e * n )      // # of comp. “extra/missed” ticks
 *  - F(n) = n + A(n)                                           // effective output ticks up to n
 *  - p(n) = frac( p + (x − 1) * n ) in [0,1)                   // residual fractional part
 *
 * Notes:
 *  - x > 1 ⇒ A increases stepwise (more output ticks than raw ticks).
 *  - x < 1 ⇒ A decreases stepwise (fewer output ticks than raw ticks).
 *
 * Conversions (mutually consistent)
 * ---------------------------------
 * Given s ≥ cos and c ≥ coc:
 *
 * 1) Clock time from simulation time (boundary aware):
 *    n  = numTicks(s − oos)
 *    n0 = numTicks(cos − oos)
 *    atBoundary = (s == oos + interval(n))
 *    m  = n − n0
 *    m  = b ? (atBoundary && m > 0 ? m − 1 : m) : m
 *    c  = coc + ( F(n0 + m) − F(n0) ) * l
 *
 * 2) Simulation time from clock time (generalized inverse):
 *    n0 = numTicks(cos − oos)
 *    k  = floor((c − coc) / l) + (b ? 0 : 1)
 *    Find m1 = min { m ≥ 0 : m + floor( p + (x − 1) * m ) ≥ k }
 *    Closed form for x > 0:
 *      m1 = ceil( (k − p) / x )
 *    n1 = n0 + max(0, m1)
 *    s  = oos + interval(n1)
 *
 * Origin moves
 * ------------
 * The origin can be moved to “now” either preserving the current clock time
 * (moveOrigin) or setting a new one (setOrigin). In both cases the mapping is
 * kept consistent by advancing the accumulator p with the raw oscillator ticks
 * that elapsed since the previous origin.
 *
 * Granularity & rounding
 * ----------------------
 * The clock can quantize output at a configured clock granularity. The optional
 * roundingFunction selects how raw values are rounded onto that grid.
 */
class INET_API OscillatorBasedClock : public ClockBase, public cListener
{
  protected:
    // Parameters
    bool useFutureEventSet = false;
    clocktime_t clockGranularity;                   // output quantization (clock time grid)
    IOscillator *oscillator = nullptr;              // driving oscillator (non-owning)
    simtime_raw_t (*roundingFunction)(simtime_raw_t, simtime_raw_t) = nullptr; // optional rounding

    // State
    /**
     * Lower bound simulation time where clock time == origin clock time (exactly at an oscillator tick).
     * Used to disambiguate boundaries when converting from s to c.
     */
    simtime_t originSimulationTimeLowerBound;

    /** Clock origin simulation time (cos): start of current mapping window. */
    simtime_t originSimulationTime;

    /** Clock origin clock time (coc): clock time at cos. */
    clocktime_t originClockTime;

    /**
     * Fractional accumulator for compensation at the clock origin.
     * Q0.63 stored modulo 2^63 in [0, 2^63). Semantics: p = frac(p + (x − 1) * n) scaled by 2^63.
     */
    uint64_t oscillatorCompensationAccumulator = 0;

    /** Clock time already accumulated from compensation (same units as clocktime_t). */
    clocktime_t clockTimeCompensation;

    /** Scheduling bookkeeping. */
    uint64_t insertionCount = 0;
    uint64_t lastNumTicks = 0;
    clocktime_t clockTimeBeforeOscillatorStateChange = -1; // assertion aid

    // Events and timers
    std::vector<ClockEvent *> events;   // pending events ordered by compareClockEvents
    cMessage *executeEventsTimer = nullptr;

  protected:
    // Lifecycle
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    // Target-module scheduling (bridge from ClockBase)
    virtual void scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event) override;
    virtual void scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event) override;
    virtual ClockEvent *cancelTargetModuleClockEvent(ClockEvent *event) override;

    /** Internal consistency checks (invariants, ordering, boundary conditions). */
    virtual void checkState() const;

    /** Verifies that all scheduled events are consistent with current mapping. */
    virtual void checkAllScheduledClockEvents() const;

    /**
     * Move the clock origin to the current simulation time without changing the
     * simulation↔clock mapping.
     *
     * Mathematical effect (s = now):
     *   n  = numTicks(s − oos)
     *   n0 = numTicks(cos − oos)
     *   m  = n − n0
     *   cos := s
     *   coc := coc + ( F(n) − F(n0) ) * l
     *   p   := frac( p + (x − 1) * m )
     */
    virtual void moveOrigin();

    /**
     * Set a new clock origin at the current simulation time with an explicit
     * clock time, keeping the mapping consistent otherwise.
     *
     * Mathematical effect (s = now, new clock time c):
     *   n  = numTicks(s − oos)
     *   n0 = numTicks(cos − oos)
     *   m  = n − n0
     *   cos := s
     *   coc := c
     *   p   := frac( p + (x − 1) * m )
     */
    virtual void setOrigin(clocktime_t clockTime);

    /**
     * Compensation helper: A(n) = floor( p + (x − 1) * (n − n0) ) = floor( p − e * (n − n0) ).
     * Uses the accumulator p at origin and the relative tick index.
     */
    int64_t A(int64_t n) const;

    /** Effective ticks: F(n) = n + A(n). */
    int64_t F(int64_t n) const { return n + A(n); }

    /**
     * Clock time from simulation time with boundary selection.
     *
     * c = doComputeClockTimeFromSimTime(s, b):
     *   n  = numTicks(s − oos)
     *   n0 = numTicks(cos − oos)
     *   atBoundary = (s == oos + interval(n))
     *   m  = n − n0
     *   m  = b ? (atBoundary && m > 0 ? m − 1 : m) : m
     *   c  = coc + ( F(n0 + m) − F(n0) ) * l
     */
    clocktime_t doComputeClockTimeFromSimTime(simtime_t t, bool lowerBound) const;

    /**
     * Simulation time from clock time with boundary selection (generalized inverse).
     *
     * s = doComputeSimTimeFromClockTime(c, b):
     *   n0 = numTicks(cos − oos)
     *   k  = floor((c − coc) / l) + (b ? 0 : 1)
     *   m1 = min { m ≥ 0 : m + floor( p + (x − 1) * m ) ≥ k }
     *   Closed form (x > 0): m1 = ceil( (k − p) / x )
     *   n1 = n0 + max(0, m1)
     *   s  = oos + interval(n1)
     */
    simtime_t doComputeSimTimeFromClockTime(clocktime_t t, bool lowerBound) const;

    /** Returns the configured clock granularity. */
    clocktime_t getClockGranularity() const { return clockGranularity; }

  public:
    virtual ~OscillatorBasedClock();

    /** Current clock time (mapping at now). */
    virtual clocktime_t getClockTime() const override;

    /** Accessor to the driving oscillator. */
    virtual const IOscillator *getOscillator() const { return oscillator; }

    /**
     * Returns the current oscillator compensation factor x (near 1). The default
     * implementation returns a neutral value; subclasses may override.
     */
    virtual SimTimeScale getOscillatorCompensation() const { return SimTimeScale(); }

    /** Conversion APIs (see formulas above). */
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t, bool lowerBound = false) const override;
    virtual simtime_t  computeSimTimeFromClockTime(clocktime_t t, bool lowerBound = true) const override;

    /** IClock scheduling (absolute “at”, relative “after”). */
    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;
    virtual bool isScheduledClockEvent(ClockEvent *event) const override;

    /** Debug/trace directive expansion. */
    virtual std::string resolveDirective(char directive) const override;

    /** React to oscillator signals (e.g., state changes) and keep mapping consistent. */
    virtual void receiveSignal(cComponent *source, int signal, uintval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, int signal, cObject *obj,      cObject *details) override;
};

} // namespace inet

#endif
