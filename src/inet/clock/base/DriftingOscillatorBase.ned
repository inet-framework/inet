//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

//
// Base module for oscillators whose effective tick rate may drift relative to a
// nominal tick length. This module provides the mapping used by clocks without
// generating one event per tick.
//
// Summary
// -------
// - Defines a computation origin `o` (o ≤ now) and a nominal tick length `L`.
// - Actual tick spacing may differ from `L` due to drift/compensation; the
//   current tick length is managed internally and may change over time.
// - Mapping operations follow ~IOscillator semantics:
//     N(Δt): number of ticks in (0, Δt] from the origin,
//     I(n) : minimal Δt ≥ 0 such that N(Δt) ≥ n.
// - When rate-related state changes, the origin and phase are updated so that
//   the first tick not earlier than the change time remains consistent (no
//   double or missed ticks).
//
// Parameters
// ----------
// - nominalTickLength (double @unit(s), default 0s):
//     Nominal tick length `L`. If set to 0s, the implementation uses the
//     simulation time precision as the nominal value. Otherwise, must be > 0.
// - tickOffset (double @unit(s), default 0s):
//     Phase offset applied at (re)initialization. Interpreted as a backward
//     shift in [0, currentTickLength). With offset δ, the first tick strictly
//     after the origin occurs at Δt = currentTickLength − δ. (δ = 0 ⇒ first
//     tick at Δt = currentTickLength.)
//
// Signals
// -------
// - preOscillatorStateChanged: emitted immediately before any change that
//   affects tick timing/mapping (e.g., rate/drift/phase/origin updates).
// - postOscillatorStateChanged: emitted immediately after such a change.
//
// Notes
// -----
// - This is a base; concrete subclasses define how drift/compensation evolve
//   (e.g., scripted changes, servo control) and how current tick length is
//   computed from those factors.
// - No per-tick events are scheduled; mapping functions are computed directly.
//
// @see ~IOscillator, ~inet.clock.contract.IClock, ~inet.clock.contract.IOscillator
//
simple DriftingOscillatorBase extends OscillatorBase
{
    parameters:
        displayStringTextFormat = default("%c (%n + %d)"); // Text shown above the submodule
        double nominalTickLength @unit(s) = default(0s);   // 0 ⇒ use simulation time precision; else > 0
        double tickOffset @unit(s) = default(0s);          // Phase shift to the past, in [0, current tick length)
        @class(DriftingOscillatorBase);
        @signal[preOscillatorStateChanged];
        @signal[postOscillatorStateChanged];
}
