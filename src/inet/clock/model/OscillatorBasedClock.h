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
bool compareClockEvents(const ClockEvent *e1, const ClockEvent *e2);

/**
 * @brief Clock model driven by an underlying IOscillator.
 *
 * Overview
 * --------
 * This clock maps between simulation time and clock time by counting oscillator ticks since an oscillator computation
 * origin and applying a (near-1) per-tick compensation. It supports both absolute 'at' and relative 'after' clock
 * scheduling (see ~IClock) without generating per-tick events.
 *
 * Invariants
 * ----------
 * 1) The clock origin simulation time (cos) is always >= the oscillator’s computation origin (oos).
 * 2) The clock origin simulation time (cos) is always <= the current simulation time (now).
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
 *  - c    : clock time argument, c >= coc
 *  - n    : numTicks(i): number of oscillator ticks in interval i >= 0
 *  - i    : interval(n): minimal simulation time to include n ticks (from oos)
 *
 * Compensation step function
 * --------------------------
 * Let e := 1 − x and define, for tick index n (from oscillator origin):
 *  - A(n) = floor(p + (x − 1) * n) = floor(p − e * n)      // number of compensated extra/missed”ticks
 *  - F(n) = n + A(n)                                       // effective output ticks up to n
 *  - p(n) = frac(p + (x − 1) * n) in [0,1)                 // residual fractional part
 *
 * Let n0 = numTicks(cos − oos) at the clock origin, p ∈ [0,1) be the residual at the origin, and m ≥ 0 the number of
 * oscillator ticks since the origin (m = n − n0). Compensation step function (relative to origin):
 *   A_rel(m) = floor(p + (x − 1) * m)
 *   F_rel(m) = m + A_rel(m)
 * Residual after m ticks: p(m) = frac(p + (x − 1) * m) ∈ [0,1)
 * For absolute indices, always take differences:
 *   F(n0 + m) − F(n0) = F_rel(m).
 *
 * Notes:
 *  - x > 1 : A increases stepwise (more output ticks than raw ticks).
 *  - x < 1 : A decreases stepwise (fewer output ticks than raw ticks).
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
 *    c  = coc + (F(n0 + m) − F(n0)) * l
 *
 * 2) Simulation time from clock time (generalized inverse):
 *    n0 = numTicks(cos − oos)
 *    k  = floor((c − coc) / l) + (b ? 0 : 1)
 *    Find m1 = min { m ≥ 0 : m + floor(p + (x − 1) * m) ≥ k }
 *    Closed-form (x > 0): m1 = max(0, ceil((k − p) / x))
 *    n1 = n0 + m1
 *    s  = oos + interval(n1)
 *
 * Origin moves
 * ------------
 * The origin can be moved to now either preserving the current clock time using moveOrigin() or setting a new one using
 * setOrigin(). In both cases the mapping is kept consistent by advancing the accumulator p with the raw oscillator ticks
 * that elapsed since the previous origin.
 */
class INET_API OscillatorBasedClock : public ClockBase, public cListener
{
  protected:
    IOscillator *oscillator = nullptr;
    int64_t (*roundingFunction)(int64_t, int64_t) = nullptr;
    bool useFutureEventSet = false;
    clocktime_t clockGranularity;

    /**
     * Lower bound simulation time where clock time == origin clock time (exactly at an oscillator tick).
     * Used to disambiguate boundaries when converting from s to c.
     */
    simtime_t originSimulationTimeLowerBound;

    /**
     * Clock origin simulation time (cos): start of current mapping window.
     */
    simtime_t originSimulationTime;

    /**
     * Clock origin clock time (coc): clock time at cos.
     */
    clocktime_t originClockTime;

    /**
     * List of clock events to be executed.
     */
    std::vector<ClockEvent *> events;

    /**
     * Fractional accumulator for compensation at the clock origin.
     * Q0.63 stored modulo 2^63 in [0, 2^63). Semantics: p = frac(p + (x − 1) * n) scaled by 2^63.
     */
    uint64_t oscillatorCompensationAccumulator = 0;

    /**
     * Clock time already accumulated from compensation (same units as clocktime_t).
     */
    clocktime_t clockTimeCompensation;

    uint64_t insertionCount = 0;
    uint64_t lastNumTicks = 0;
    clocktime_t clockTimeBeforeOscillatorStateChange = -1;

    cMessage *executeEventsTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event) override;
    virtual void scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event) override;
    virtual ClockEvent *cancelTargetModuleClockEvent(ClockEvent *event) override;

    virtual void checkState() const;
    virtual void checkAllScheduledClockEvents() const;

    // frac in Q0.63: keep modulo 2^63
    uint64_t fracAdvance(int64_t m) const;

    // floor_q63(p - e * m)
    int64_t Arel(int64_t m) const;

    /**
     * Compensation helper: A(n) = floor( p + (x − 1) * (n − n0) ) = floor( p − e * (n − n0) ).
     * Uses the accumulator p at origin and the relative tick index.
     */
    int64_t A(int64_t n) const;

    // Effective ticks: F(n) = n + A(n)
    int64_t F(int64_t n) const { return n + A(n); }

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
     *   Closed-form (x > 0): m1 = max(0, ceil((k − p) / x))
     *   n1 = n0 + m1
     *   s  = oos + interval(n1)
     */
    simtime_t doComputeSimTimeFromClockTime(clocktime_t t, bool lowerBound) const;

    clocktime_t getClockGranularity() const { return clockGranularity; }

  public:
    virtual ~OscillatorBasedClock();

    virtual clocktime_t getClockTime() const override;

    virtual const IOscillator *getOscillator() const { return oscillator; }

    /**
     * Returns the current oscillator compensation factor x (near 1). The default
     * implementation returns a neutral value; subclasses may override.
     */
    virtual SimTimeScale getOscillatorCompensation() const { return SimTimeScale(); }

    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t, bool lowerBound = false) const override;
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
