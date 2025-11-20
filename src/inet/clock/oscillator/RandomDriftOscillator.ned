//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.oscillator;

import inet.clock.base.DriftingOscillatorBase;
import inet.clock.contract.IOscillator;

//
// Oscillator with time-varying drift driven by a bounded random walk.
//
// Summary
// -------
// The oscillator’s fractional frequency error (drift rate, in ppm) evolves over
// time. At the end of each `changeInterval`, an increment Δr (sampled from the
// volatile parameter `driftRateChange`) is added to the current drift rate,
// optionally clamped to [driftRateChangeLowerLimit, driftRateChangeUpperLimit].
// This produces a bounded random walk starting from `initialDriftRate`.
//
// Semantics
// ---------
// Let r_k be the drift rate (ppm) right after the k-th update (k ≥ 0), with
// r_0 = initialDriftRate. At each update time t_k:
//   Δr_k  := sample(driftRateChange)          // volatile; re-evaluated each update
//   r_k+1 := clamp(r_k + Δr_k,
//                  driftRateChangeLowerLimit,
//                  driftRateChangeUpperLimit)
// The effective tick-length factor is d ≈ 1 + r·1e−6, and the current tick
// length is L_current = L / d (L = nominal tick length). Phase/origin handling
// follows ~DriftingOscillatorBase: updates preserve tick phase (no double/missed
// ticks) by moving the computation origin to the change instant.
//
// Timing of updates
// -----------------
// - Updates occur at the end of every `changeInterval`. Because it is declared
//   `volatile`, `changeInterval` may be an expression/distribution; its value is
//   (re)sampled whenever a new update is scheduled.
// - The drift-change event is scheduled with priority `driftChangeEventSchedulingPriority`.
//
// Parameters
// ----------
// - initialDriftRate (@unit(ppm), default 0ppm):
//     Starting drift rate r_0.
// - changeInterval (volatile @unit(s)):
//     Time between successive drift updates; re-evaluated after each update.
// - driftRateChange (volatile @unit(ppm), default 0ppm):
//     Increment distribution Δr applied at each update (random-walk step).
// - driftRateChangeLowerLimit (@unit(ppm), default −inf):
//     Lower clamp on the *accumulated* drift rate.
// - driftRateChangeUpperLimit (@unit(ppm), default +inf):
//     Upper clamp on the *accumulated* drift rate.
// - driftChangeEventSchedulingPriority (int, default 1000):
//     Scheduling priority for the internal drift-change event.
//
// Notes
// -----
// - The mapping N(Δt)/I(n) and origin/phase invariants are implemented by the
//   base module; this module only drives the drift process.
// - Use `tickOffset` (inherited) to set the initial phase relative to origin.
//
// @see ~IOscillator, ~inet.clock.base.DriftingOscillatorBase
//
simple RandomDriftOscillator extends DriftingOscillatorBase like IOscillator
{
    parameters:
        double initialDriftRate @unit(ppm) = default(0ppm);        // Initial drift r_0
        volatile double changeInterval @unit(s);                   // Interval between updates (re-sampled)
        volatile double driftRateChange @unit(ppm) = default(0ppm);// Random-walk increment Δr (re-sampled)
        double driftRateChangeLowerLimit @unit(ppm) = default(-inf ppm); // Clamp on accumulated drift
        double driftRateChangeUpperLimit @unit(ppm) = default(inf ppm);  // Clamp on accumulated drift
        int driftChangeEventSchedulingPriority = default(1000);    // Priority of update event
        @class(RandomDriftOscillator);
}
