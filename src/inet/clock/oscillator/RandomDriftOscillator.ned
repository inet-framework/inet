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
simple RandomDriftOscillator extends DriftingOscillatorBase like IOscillator
{
    parameters:
        double initialDriftRate @unit(ppm) = default(0ppm); // Expressed as a ratio in parts per million
        volatile double changeInterval @unit(s); // Drift change happens at the end of every interval
        volatile double driftRateChange @unit(ppm) = default(0ppm); // Integrated over time (random walk), no change by default
        double driftRateChangeLowerLimit @unit(ppm) = default(-inf ppm); // Lower limit for random walk, no limit by default
        double driftRateChangeUpperLimit @unit(ppm) = default(inf ppm); // Upper limit for random walk, no limit by default
        int driftChangeEventSchedulingPriority = default(1000); // Specifies the scheduling priority of the drift change event
        @class(RandomDriftOscillator);
}

