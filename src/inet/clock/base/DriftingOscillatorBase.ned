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
// - Defines a computation origin o (o ≤ now) and a nominal tick length L.
// - Actual tick spacing may differ from L due to drift/compensation; the
//   current tick length is managed internally and may change over time.
// - Mapping operations follow ~IOscillator semantics:
//     N(Δt): number of ticks in (0, Δt] from the origin,
//     I(n) : minimal Δt ≥ 0 such that N(Δt) ≥ n.
// - When rate-related state changes, the origin and phase are updated so that
//   the first tick not earlier than the change time remains consistent (no
//   double or missed ticks).
//
simple DriftingOscillatorBase extends OscillatorBase
{
    parameters:
        displayStringTextFormat = default("%c (%n + %d)"); // Determines the text that is written on top of the submodule
        double nominalTickLength @unit(s); // Specifies the elapsed time between oscillator ticks
        double tickOffset @unit(s) = default(0s); // Shifts ticks to the past, must be in the range of [0, current tick length)
        @signal[preOscillatorStateChanged];
        @signal[postOscillatorStateChanged];
        @class(DriftingOscillatorBase);
}

