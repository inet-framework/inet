//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.oscillator;

import inet.clock.base.OscillatorBase;
import inet.clock.contract.IOscillator;

//
// Ideal strictly periodic oscillator (no drift).
//
// Summary
// -------
// Provides a constant tick length L and implements the ~IOscillator mappings
// without generating per-tick events.
//
// Semantics
// ---------
// - Let o be the computation origin (o ≤ now), and L = tickLength (> 0 unless 0s is used; see below).
// - Tick instants (relative to origin): τ_k = k·L for k = 1,2,… (tick at origin is not counted).
// - Mapping functions:
//     N(Δt) = floor(Δt / L)            // ticks in (0, Δt], Δt ≥ 0
//     I(n)  = n · L, with I(0) = 0     // minimal Δt for n ticks, n ≥ 0
//
// @see ~IOscillator
//
simple IdealOscillator extends OscillatorBase like IOscillator
{
    parameters:
        double tickLength @unit(s) = default(0s); // Constant tick length L. If 0s, the simulation time precision is used; otherwise must be > 0.
        @class(IdealOscillator);
}
