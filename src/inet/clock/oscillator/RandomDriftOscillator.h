//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RANDOMDRIFTOSCILLATOR_H
#define __INET_RANDOMDRIFTOSCILLATOR_H

#include "inet/clock/base/DriftingOscillatorBase.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * @brief Oscillator whose drift rate follows a (bounded) random walk.
 *
 * Overview
 * --------
 * The oscillator’s fractional frequency error (drift rate, in ppm) evolves over
 * time. At the end of each update interval, a drift increment Δr (sampled from
 * a parameter/expression) is added to the current drift, optionally clamped to
 * configured lower/upper limits. After each change, the effective tick length
 * and phase are updated via DriftingOscillatorBase while preserving the “no
 * double/no missed tick” invariant (by moving the computation origin to the
 * change time and recomputing the first-tick phase).
 *
 * Update law
 * ----------
 * Let r_k be the drift rate (ppm) immediately AFTER the k-th update (k ≥ 0), with
 * r_0 taken from `initialDriftRate`. At each update time t_k:
 *
 *   Δr_k  := sample(driftRateChangeParameter)   // ppm, may be volatile
 *   r_{k+1} := clamp(r_k + Δr_k,
 *                    driftRateChangeLowerLimit,
 *                    driftRateChangeUpperLimit)
 *
 * Effective tick-length factor d ≈ 1 + r·1e−6, current tick length L_cur = L / d,
 * where L is the nominal tick length. Origin/phase handling is done by the base.
 *
 * Timing
 * ------
 * Update times are driven by a self-message (`changeTimer`). The delay to the
 * next update is sampled from `changeIntervalParameter` each time the timer is
 * (re)scheduled (supports volatile/distribution expressions). The scheduling
 * priority is controlled by the NED parameter declared on the corresponding
 * simple module (see RandomDriftOscillator.ned).
 *
 * Parameters (read in initialize())
 * ---------------------------------
 * - initialDriftRate (ppm): starting drift rate r_0.
 * - driftRateChangeParameter (ppm): increment distribution for Δr.
 * - changeIntervalParameter (s): interval between updates (sampled each step).
 * - driftRateChangeLowerLimit / UpperLimit (ppm): clamps on the accumulated drift.
 *
 * Notes
 * -----
 * - No per-tick events are generated; mapping N(Δt)/I(n) is computed analytically.
 * - Destructor cancels and deletes the self-message.
 * - Signals about rate changes are emitted by the base when setDriftRate() updates d.
 */
class INET_API RandomDriftOscillator : public DriftingOscillatorBase
{
  protected:
    /** Self-message that triggers periodic drift updates. */
    cMessage *changeTimer = nullptr;

    /** Starting drift r_0 (ppm); read from NED parameter during initialize(). */
    ppm initialDriftRate = ppm(NaN);

    /** Parameter supplying Δr (ppm) at each update; may be a distribution/expression. */
    cPar *driftRateChangeParameter = nullptr;

    /** Parameter supplying the time to the next update; re-sampled each step. */
    cPar *changeIntervalParameter = nullptr;

    /** Accumulated random-walk component ΣΔr (ppm) since start; informational. */
    ppm driftRateChangeTotal = ppm(0);

    /** Lower clamp on accumulated drift (ppm); NaN ⇒ no lower limit. */
    ppm driftRateChangeLowerLimit = ppm(NaN);

    /** Upper clamp on accumulated drift (ppm); NaN ⇒ no upper limit. */
    ppm driftRateChangeUpperLimit = ppm(NaN);

  protected:
    /** Cancels and deletes the self-message if allocated. */
    virtual ~RandomDriftOscillator() { cancelAndDelete(changeTimer); }

    /**
     * Reads parameters, initializes drift state (r_0), allocates `changeTimer`,
     * schedules the first update based on `changeIntervalParameter`, and applies
     * the initial drift via setDriftRate().
     */
    virtual void initialize(int stage) override;

    /**
     * Handles `changeTimer`: samples Δr and the next interval, updates the drift
     * via setDriftRate(), updates cumulative statistics, and reschedules the timer.
     * Other messages are passed to the base if applicable.
     */
    virtual void handleMessage(cMessage *message) override;
};

} // namespace inet

#endif
