//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DRIFTINGOSCILLATORBASE_H
#define __INET_DRIFTINGOSCILLATORBASE_H

#include "inet/clock/base/OscillatorBase.h"
#include "inet/common/INETMath.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/common/SimTimeScale.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * @brief Oscillator with piecewise-constant rate (drift) and optional frequency compensation.
 *
 * Mathematical notation:
 *  - o : computation origin [s], o >= 0
 *  - l : nominal tick length [s], l > 0
 *  - d : physical drift factor (near 1), d > 0
 *  - f : frequency compensation factor (near 1), f > 0
 *  - g = d * f : effective tick-length factor (near 1), g > 0
 *  - n : tick count from origin (cardinality), n >= 0
 *  - t : interval from origin [s], t >= 0
 *  - x : time of the first tick strictly after the origin [s], 0 < x <= l/g
 *
 * Tick semantics (g is piecewise-constant between origin updates):
 *  - Ticks occur at absolute times: o + x + k * (l / g), for k = 0,1,2,…
 *  - Tick count from the origin for Δt >= 0:
 *      N(Δt) = |{ k : 0 < x + k * (l / g) <= Δt }|
 *             = 0                       if Δt == 0
 *             = floor( g * (Δt - x) / l ) + 1   otherwise
 *  - Minimal interval for n ticks (generalized right-inverse of N):
 *      I(0)  = 0
 *      I(n)  = x + (n - 1) * (l / g)           for n > 0
 *
 * Origin update rule:
 *  - When d or f changes at simulation time s, a new computation origin is set:
 *      o := s
 *      g := d * f (new effective factor)
 *      Choose x such that the first tick not earlier than s becomes o + x
 *      (i.e., preserve tick phase—no double/missed ticks). Invariant: 0 < x <= l/g.
 *  - numTicksAtOrigin is increased by N(s - old_o) computed with the *previous* g.
 *
 * Rates and units:
 *  - driftRate (ppm): Δf_phys_ppm = 10^6 * (f_this - f_nominal) / f_nominal.
 *      driftRate > 0 ⇒ oscillator runs faster ⇒ shorter current tick length.
 *      Approx.: d ≈ 1 + driftRate * 1e-6.
 *  - frequencyCompensationRate (ppm): software-applied correction; f ≈ 1 + rate * 1e-6.
 *  - Current tick length is l_current = l / g = l / (d * f).
 */
class INET_API DriftingOscillatorBase : public OscillatorBase, public IScriptable
{
  public:
    static simsignal_t driftRateChangedSignal;
    static simsignal_t frequencyCompensationRateChangedSignal;

  protected:
    simtime_t nominalTickLength;
    ppm inverseDriftRate = ppm(NaN); // stored for faster computation, calculated as (1 / (1 + driftRate / 1E+6) - 1) * 1E-6

    /**
     * Physical frequency error in ppm.
     *   0 ppm  ⇒ nominal,
     *  >0 ppm  ⇒ faster (gains time; shorter ticks),
     *  <0 ppm  ⇒ slower (loses time; longer ticks).
     */
    ppm driftRate = ppm(NaN);

    /**
     * Dimensionless factor near 1 representing the ratio nominal/current tick length
     * due to physical drift only (d). For positive driftRate, driftFactor > 1.
     * Relation (approx.): driftFactor ≈ 1 + driftRate * 1e-6.
     */
    SimTimeScale driftFactor;

    /** Software/servo-applied frequency correction in ppm (0 ⇒ no compensation). */
    ppm frequencyCompensationRate = ppm(0);

    /**
     * Dimensionless factor near 1 modeling the fine-tuning compensation only (f).
     * Relation (approx.): frequencyCompensationFactor ≈ 1 + frequencyCompensationRate * 1e-6.
     */
    SimTimeScale frequencyCompensationFactor;

    /**
     * Effective tick-length factor g = d * f (dimensionless, near 1).
     * Current tick length is l_current = l / g.
     */
    SimTimeScale effectiveTickLengthFactor;

    /**
     * Computation origin o. All mapping functions are measured from this time.
     * Invariant: origin <= current simulation time; origin need not coincide with a tick.
     */
    simtime_t origin;

    /**
     * Phase x: time from origin to the first tick strictly after origin.
     * Invariant: 0 < nextTickFromOrigin <= l_current.
     */
    simtime_t nextTickFromOrigin;

    /** Total number of ticks strictly before the origin (absolute tick index at o). */
    int64_t numTicksAtOrigin;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleTickTimer() override;
    virtual void scheduleTickTimer() override;

    /**
     * Set a new computation origin (o := newOrigin) while preserving tick phase:
     *   - Increase numTicksAtOrigin by N(newOrigin - oldOrigin) under the *old* g.
     *   - Recompute nextTickFromOrigin so the first tick >= newOrigin becomes o + x.
     *   - Keep 0 < x <= l_current.
     */
    virtual void setOrigin(simtime_t origin);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

    ppm invertDriftRate(ppm driftRate) const { return unit(1 / (1 + driftRate.get<unit>()) - 1); }
    virtual void setDriftFactor(SimTimeScale driftFactor);

    virtual void setFrequencyCompensationFactor(SimTimeScale frequencyCompensationFactor);

    virtual void setEffectiveTickLengthFactor(SimTimeScale effectiveTickLengthFactor);
    int64_t increaseWithDriftRate(int64_t value) const { return increaseWithDriftRate(value, driftRate); }
    int64_t increaseWithDriftRate(int64_t value, ppm driftRate) const { return value + (int64_t)(value * driftRate.get<unit>()); }

    int64_t decreaseWithDriftRate(int64_t value) const { return decreaseWithDriftRate(value, inverseDriftRate); }
    int64_t decreaseWithDriftRate(int64_t value, ppm inverseDriftRate) const { return value + (int64_t)(value * inverseDriftRate.get<unit>()); }
    int64_t doComputeTicksForInterval(simtime_t timeInterval) const;
    simtime_t doComputeIntervalForTicks(int64_t numTicks) const;

  public:
    virtual simtime_t getComputationOrigin() const override { return origin; }
    virtual simtime_t getNominalTickLength() const override { return nominalTickLength; }

    virtual simtime_t getCurrentTickLength() const;

    /** Returns the absolute tick count at the origin (ticks strictly before o). */
    virtual int64_t getNumTicksAtOrigin() const override { return numTicksAtOrigin; }
    virtual ppm getDriftRate() const { return driftRate; }
    virtual void setDriftRate(ppm driftRate);

    virtual ppm getFrequencyCompensationRate() const { return frequencyCompensationRate; }
    virtual void setFrequencyCompensationRate(ppm frequencyCompensationRate);

    /**
     * Set tick phase as a backward shift δ in [0, L_current):
     *   x := L_current − (δ mod L_current); if result is 0, use L_current.
     * The first tick strictly after the origin then occurs at o + x.
     * (δ = 0 ⇒ x = L_current; no tick exactly at the origin.)
     */
    virtual void setTickOffset(simtime_t tickOffset);

    /**
     * Number of ticks in (0, Δt] from the origin:
     *   N(Δt) = 0 if Δt == 0
     *         = floor( g * (Δt - x) / l ) + 1 otherwise,
     * where g = d * f and x is the first-tick phase.
     */
    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;

    /**
     * Minimal interval length for n ticks:
     *   I(0) = 0
     *   I(n) = x + (n - 1) * (l / g) for n > 0,
     * where g = d * f and x is the first-tick phase.
     */
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif
