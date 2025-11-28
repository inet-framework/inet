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
 * Provides a strictly periodic tick sequence with constant tick length and no
 * drift. The computation origin is a simulation time (o ≤ now).
 *
 * Mathematical semantics
 * ----------------------
 * - Let L := tickLength > 0 be the nominal tick interval (constant).
 * - Tick instants (relative to origin) are τ_i := i·L, i = 1,2,…  (the origin
 *   itself is not counted as a tick).
 * - Tick count from origin for interval Δt ≥ 0: N(Δt) = |{ i : 0 < τ_i ≤ Δt }|
 *   = floor(Δt / L) (tick exactly at Δt is included; a tick at the origin is
 *   excluded).
 * - Minimal interval for n ≥ 0 ticks: I(n) = inf{ Δt ≥ 0 : N(Δt) ≥ n } = n · L
 *   with I(0) = 0.
 *
 * This class implements the ~IOscillator mapping without generating per-tick
 * events. It is suitable as a reference oscillator for clocks that require a
 * simple, drift-free timebase.
 */
class INET_API IdealOscillator : public OscillatorBase
{
  protected:
    /// Computation origin (o ≤ now); tick counting is measured from here.
    simtime_t origin;

    /// Constant tick length L > 0 used for both N(Δt) and I(n).
    simtime_t tickLength;

  protected:
    virtual void initialize(int stage) override;
    virtual void scheduleTickTimer() override;

  public:
    /** @name IOscillator contract */
    ///@{
    virtual simtime_t getComputationOrigin() const override { return origin; }

    virtual simtime_t getNominalTickLength() const override { return tickLength; }

    virtual int64_t getNumTicksAtOrigin() const override { return 0; }

    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;

    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;
    ///@}
};

} // namespace inet

#endif
