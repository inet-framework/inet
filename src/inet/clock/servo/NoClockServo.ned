//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

import inet.clock.base.ClockServoBase;
import inet.clock.contract.IClockServo;

//
// No-op clock servo.
//
// Summary
// -------
// Implements ~IClockServo but intentionally performs no adjustments to the
// controlled clock. Useful as a baseline, for A/B comparisons, or to disable
// disciplining while keeping the servo plumbing and measurements intact.
//
// Semantics
// ---------
// - Ignores all offset/rate measurements.
// - Never steps or slews the clock; the clock remains free-running.
// - Preserves all ~IClock invariants trivially (no changes are applied).
//
// Parameters
// ----------
// - clockModule (inherited from ~ClockServoBase): path to the clock to be (not) disciplined.
//
// @see ~inet.clock.base.ClockServoBase, ~inet.clock.contract.IClockServo, ~inet.clock.contract.IClock
//
module NoClockServo extends ClockServoBase like IClockServo
{
    parameters:
        @class(NoClockServo);
}
