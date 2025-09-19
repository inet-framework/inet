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
 *
 * Signals:
 *  - driftRateChangedSignal: emitted after driftRate (and thus d) is updated.
 *  - frequencyCompensationRateChangedSignal: emitted after frequencyCompensationRate (and thus f) is updated.
 *
 * Notes:
 *  - This class implements the ~IOscillator mapping without per-tick events.
 *  - scheduleTickTimer()/handleTickTimer() exist for internal housekeeping or scenario integration;
 *    they must NOT generate one event per tick.
 */
class INET_API DriftingOscillatorBase : public OscillatorBase, public IScriptable
{
  public:
    static simsignal_t driftRateChangedSignal;
    static simsignal_t frequencyCompensationRateChangedSignal;

  protected:
    /** Nominal tick length l (fixed). */
    simtime_t nominalTickLength;

    /**
     * Physical frequency error in ppm.
     *   0 ppm  ⇒ nominal,
     *  >0 ppm  ⇒ faster (gains time; shorter ticks),
     *  <0 ppm  ⇒ slower (loses time; longer ticks).
     * Example: +100 ppm ≈ gains 100 µs per second.
     *
     * Implementation note: initialized to NaN to signal “unset” before initialize().
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
    /** Initialize parameters, state, and factors (l, d, f, g, x, counters). */
    virtual void initialize(int stage) override;
    virtual void finish() override;

    /**
     * Internal timer callback (no per-tick events). Implementations may use this
     * to react to scripted changes or housekeeping that cannot be done immediately.
     */
    virtual void handleTickTimer() override;

    /** (Re)arm any internal timer used by the base; must not represent per-tick scheduling. */
    virtual void scheduleTickTimer() override;

    /**
     * Set a new computation origin (o := newOrigin) while preserving tick phase:
     *   - Increase numTicksAtOrigin by N(newOrigin - oldOrigin) under the *old* g.
     *   - Recompute nextTickFromOrigin so the first tick >= newOrigin becomes o + x.
     *   - Keep 0 < x <= l_current.
     */
    virtual void setOrigin(simtime_t origin);

    /** Update d (from driftRate) and recompute g; triggers origin update if required. */
    virtual void setDriftFactor(SimTimeScale driftFactor);

    /** Update f (from frequencyCompensationRate) and recompute g; triggers origin update if required. */
    virtual void setFrequencyCompensationFactor(SimTimeScale frequencyCompensationFactor);

    /** Set g directly (typically from d and f) and maintain invariants (x, counters). */
    virtual void setEffectiveTickLengthFactor(SimTimeScale effectiveTickLengthFactor);

    // IScriptable
    /** Handle scenario directives (e.g., scripted drift/compensation changes). */
    virtual void processCommand(const cXMLElement& node) override;

    /** Core math with g held constant (no bounds/parameter checks). */
    int64_t doComputeTicksForInterval(simtime_t timeInterval) const;
    /** Core math with g held constant (no bounds/parameter checks). */
    simtime_t doComputeIntervalForTicks(int64_t numTicks) const;

  public:
    /** @name IOscillator contract and extensions */
    ///@{

    /** Returns the computation origin o. */
    virtual simtime_t getComputationOrigin() const override { return origin; }

    /** Returns the nominal tick length l. */
    virtual simtime_t getNominalTickLength() const override { return nominalTickLength; }

    /** Returns the current (effective) tick length l_current = l / (d * f). */
    virtual simtime_t getCurrentTickLength() const;

    /** Returns the absolute tick count at the origin (ticks strictly before o). */
    virtual int64_t getNumTicksAtOrigin() const override { return numTicksAtOrigin; }

    /** Physical drift in ppm (sign: + faster, − slower). */
    virtual ppm getDriftRate() const { return driftRate; }
    /** Set physical drift in ppm; updates d (and g) and emits driftRateChangedSignal. */
    virtual void setDriftRate(ppm driftRate);

    /** Software compensation in ppm (sign: + speeds up, − slows down). */
    virtual ppm getFrequencyCompensationRate() const { return frequencyCompensationRate; }
    /** Set compensation in ppm; updates f (and g) and emits frequencyCompensationRateChangedSignal. */
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

    ///@}

    /** Resolve printf-like directive characters used in debug/trace output. */
    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif
