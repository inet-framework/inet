//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEALOSCILLATOR_H
#define __INET_IDEALOSCILLATOR_H

#include "inet/clock/base/OscillatorBase.h"
#include "inet/common/INETMath.h"

namespace inet {

/**
 * @brief Ideal (uniform) oscillator.
 *
 * Provides a strictly periodic tick sequence with constant tick length and
 * no drift. The computation origin `o` is a simulation time (o ≤ now).
 *
 * Mathematical semantics
 * ----------------------
 * - Let L := tickLength > 0 be the nominal tick interval (constant).
 * - Tick instants (relative to origin) are τ_i := i·L, i = 1,2,…  (the origin
 *   itself is not counted as a tick).
 * - Tick count from origin for interval Δt ≥ 0:
 *       N(Δt) = |{ i : 0 < τ_i ≤ Δt }| = floor(Δt / L)
 *   (tick exactly at Δt is included; a tick at the origin is excluded).
 * - Minimal interval for n ≥ 0 ticks:
 *       I(n) = inf{ Δt ≥ 0 : N(Δt) ≥ n } = n · L
 *   with I(0) = 0.
 *
 * Notes
 * -----
 * - This class implements the ~IOscillator mapping without generating per-tick
 *   events. It is suitable as a reference oscillator for clocks that require a
 *   simple, drift-free timebase.
 */
class INET_API IdealOscillator : public OscillatorBase
{
  protected:
    /// Computation origin `o` (o ≤ now); tick counting is measured from here.
    simtime_t origin;

    /// Constant tick length L > 0 used for both N(Δt) and I(n).
    simtime_t tickLength;

  protected:
    /** @internal Initialize origin and tick length from parameters; chain to base. */
    virtual void initialize(int stage) override;

    /**
     * @internal Implementation hook used by OscillatorBase to (re)arm any timers.
     * IdealOscillator does not schedule per-tick events; this is only for internal
     * housekeeping if the base expects a timer.
     */
    virtual void scheduleTickTimer() override;

  public:
    /** @name IOscillator contract */
    ///@{

    /** Returns the computation origin `o`. */
    virtual simtime_t getComputationOrigin() const override { return origin; }

    /** Returns the nominal (and actual) tick length L. */
    virtual simtime_t getNominalTickLength() const override { return tickLength; }

    /**
     * Returns the total number of ticks at the origin. For the ideal model this
     * is 0 (ticks start strictly after the origin).
     */
    virtual int64_t getNumTicksAtOrigin() const override { return 0; }

    /**
     * Computes N(Δt) for Δt := timeInterval ≥ 0:
     *   N(Δt) = floor(Δt / L).
     * A tick exactly at Δt is counted; a tick at the origin is not.
     */
    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;

    /**
     * Computes I(n) for n := numTicks ≥ 0:
     *   I(0) = 0,  I(n) = n · L for n > 0.
     */
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;

    ///@}
};

} // namespace inet

#endif
