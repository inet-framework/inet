//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IOSCILLATOR_H
#define __INET_IOSCILLATOR_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * @brief Interface for oscillators.
 *
 * An oscillator produces conceptual “tick” instants. It exposes a nominal tick
 * interval as a reference, but the actual tick timing may deviate (e.g., due to
 * drift or phase). For performance, the oscillator does not generate an event
 * per tick; instead it provides two mapping functions that translate between a
 * time interval and the number of ticks within that interval, both measured
 * from a computation origin.
 *
 * Computation origin:
 * - The computation origin o is a simulation time (o <= now) from which tick
 *   counting is defined. The origin need not coincide with a tick; i.e., the
 *   first tick after the origin may occur strictly later.
 *
 * Mathematical semantics:
 * - Let {τ_i | i = 1,2,...} be the strictly increasing sequence of tick times
 *   after the origin (relative to o), so each absolute tick time is o + τ_i.
 * - Define the step function (tick count from origin):
 *     N(Δt) := |{ i : 0 < τ_i <= Δt }|, for Δt >= 0.
 *   (Ticks at the origin are not counted; a tick exactly at Δt is counted.)
 * - Define the generalized inverse (minimal interval for n ticks):
 *     I(n) := inf { Δt >= 0 : N(Δt) >= n }, for n >= 0.
 *   (By convention I(0) = 0.)
 *
 * Properties:
 * - N is non-decreasing and right-continuous, with unit jumps at tick instants.
 * - I is the right-inverse of N: N(I(n)) >= n and I(N(Δt)) <= Δt.
 * - Monotonicity:
 *     Δt1 <= Δt2  ⇒ N(Δt1) <= N(Δt2),
 *     n1  <= n2   ⇒ I(n1)  <= I(n2).
 * - Nominal tick length returned by getNominalTickLength() is a fixed reference
 *   parameter; it does not imply actual tick spacing is constant.
 *
 * Signals:
 * - preOscillatorStateChangedSignal: emitted immediately before any change to
 *   oscillator parameters that affect tick timing or mapping (e.g., drift/rate,
 *   phase/offset, or origin).
 * - postOscillatorStateChangedSignal: emitted immediately after such a change.
 *
 * Invariants:
 * - getComputationOrigin() <= current simulation time.
 */
class INET_API IOscillator
{
  public:
    /// Emitted immediately before a change that affects tick timing/mapping.
    static simsignal_t preOscillatorStateChangedSignal;
    /// Emitted immediately after a change that affects tick timing/mapping.
    static simsignal_t postOscillatorStateChangedSignal;

  public:
    virtual ~IOscillator() {}

    /**
     * @brief Returns the nominal tick interval (a fixed reference quantity).
     *
     * This value is a configuration/parameter reference and does not by itself
     * guarantee equal spacing of actual ticks.
     */
    virtual simtime_t getNominalTickLength() const = 0;

    /**
     * @brief Returns the total number of ticks at the computation origin.
     *
     * This is the count accumulated strictly before the origin (useful if the
     * oscillator maintains an absolute tick index). It does not affect the
     * mapping N/I defined from the origin onward.
     */
    virtual int64_t getNumTicksAtOrigin() const = 0;

    /**
     * @brief Returns the computation origin o (o <= now).
     *
     * Tick counting for the mapping functions is measured from this time.
     * A tick may occur exactly at the origin, but it is not counted by N.
     */
    virtual simtime_t getComputationOrigin() const = 0;

    /**
     * @brief Computes the number of ticks within (0, Δt] from the origin.
     *
     * Implements N(Δt) with Δt = timeInterval >= 0:
     * - N(0) = 0.
     * - A tick exactly at Δt is included; a tick at the origin is excluded.
     *
     * @param timeInterval  Δt >= 0, measured from getComputationOrigin().
     * @return              N(Δt), the number of ticks in (0, Δt].
     */
    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const = 0;

    /**
     * @brief Computes the minimal interval length needed to include `n` ticks.
     *
     * Implements I(n) = inf { Δt >= 0 : N(Δt) >= n } with n >= 0:
     * - I(0) = 0.
     * - For n > 0, I(n) is the time of the n-th tick after the origin, measured
     *   from the origin, with a tick exactly at the boundary counted as included.
     *
     * @param numTicks  n >= 0.
     * @return          I(n), the minimal Δt such that N(Δt) >= n.
     */
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const = 0;
};

} // namespace inet

#endif

