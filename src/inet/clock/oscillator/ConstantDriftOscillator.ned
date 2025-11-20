//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.oscillator;

import inet.clock.base.DriftingOscillatorBase;
import inet.clock.contract.IOscillator;

//
// Oscillator with constant fractional frequency offset (drift).
//
// Summary
// -------
// Produces a strictly periodic tick sequence whose rate is a constant factor
// faster or slower than the nominal tick length. No per-tick events are
// scheduled; mappings follow ~IOscillator semantics via the base.
//
// Semantics
// ---------
// - Let L be the nominal tick length and r be `driftRate` in ppm.
// - Effective factor d = 1 + r·1e−6.
// - Current tick length: L_current = L / d.
//   * r > 0  → faster oscillator (shorter ticks)
//   * r = 0  → nominal rate
//   * r < 0  → slower oscillator (longer ticks)
// - Phase is controlled by `tickOffset` inherited from the base: the first tick
//   strictly after the origin occurs at Δt = L_current − δ, where
//   δ = tickOffset reduced into [0, L_current).
//
// Runtime control
// ---------------
// - The drift can be set from C++ or via ScenarioManager:
//     <set-oscillator module="path.to.osc" drift-rate="100ppm" tick-offset="0.1ms"/>
//   Changes take effect by updating the computation origin and preserving phase
//   (no double or missed ticks).
//
// Parameters
// ----------
// - driftRate (@unit(ppm), default 0ppm): constant fractional frequency error.
//   Positive = faster, negative = slower.
//
// @see ~IOscillator, ~inet.clock.base.DriftingOscillatorBase, ~ScenarioManager
//
simple ConstantDriftOscillator extends DriftingOscillatorBase like IOscillator
{
    parameters:
        double driftRate @unit(ppm) = default(0ppm); // Constant fractional frequency offset
        @class(ConstantDriftOscillator);
}
