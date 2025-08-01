//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

import inet.common.Module;

//
// Base module for clock servos.
//
// Summary
// -------
// Provides common wiring/parameters for modules that implement a clock servo
// (see ~inet.clock.contract.IClockServo). A servo disciplines a local clock
// (~inet.clock.contract.IClock) based on measured offset/rate differences
// from a reference source. Concrete algorithms (PI/PLL/FLL, etc.) are
// implemented in subclasses.
//
// Parameters
// ----------
// - clockModule (string):
//     Module path to the controlled clock (implements ~IClock). Typical forms:
//       "^.clock"   // sibling submodule
//       "host.clock"
//     Derived servos resolve and use this clock to apply offset/rate adjustments.
//
// Notes
// -----
// - This base does not define the control law or measurement inputs; subclasses
//   should obtain measurements (e.g., from a sync protocol) and call their
//   IClockServo implementation to steer the clock.
// - Display icon: control block.
//
// @see ~inet.clock.contract.IClock, ~inet.clock.contract.IClockServo
//
module ClockServoBase extends Module
{
    parameters:
        string clockModule;         // Path to the controlled clock module (IClock)
        @class(ClockServoBase);
        @display("i=block/control");
}
