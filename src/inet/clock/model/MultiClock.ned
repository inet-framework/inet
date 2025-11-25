//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.common.Module;
import inet.clock.contract.IClock;

//
// Multi-clock aggregator and switch.
//
// Summary
// -------
// Exposes a single ~IClock interface while internally hosting multiple
// subclocks. All IClock operations (conversions, scheduling) are delegated to
// the *currently active* subclock selected by `activeClockIndex`. The active
// clock can be changed at runtime (e.g., to switch gPTP time domains).
//
// Semantics
// ---------
// - Delegation: getClockTime(), scheduleClockEventAt/After(), cancel, and the
//   conversion helpers are forwarded to clock[activeClockIndex] at the moment
//   the call is made.
// - Switching: updating `activeClockIndex` changes *future* delegations only.
//   It does not retroactively migrate already scheduled events; those remain
//   owned/managed by the subclock that scheduled them.
// - Time continuity: switching may introduce a discontinuity in the exposed
//   clock time if subclocks are not aligned. Client modules that assume a
//   monotone clock should handle such discontinuities (e.g., via lower/upper
//   bound handling or by listening for `timeChanged`).
//
// Parameters
// ----------
// - numClocks (int):
//     Number of subclocks to instantiate (array size of `clock[]`).
// - activeClockIndex (int, @mutable, default 0):
//     Index of the currently active subclock in [0, numClocks-1]. Changing this
//     at runtime switches the delegation target as described above.
//
// Signals / Statistics
// --------------------
// - timeChanged (simtime_t):
//     Emitted by this module when the exposed clock time changes due to a
//     control action (e.g., active-clock switch). The exact emission policy is
//     implementation-specific.
// - Statistic `timeChanged` records the signal as a vector.
//
// Typical use
// -----------
// - Multi-domain gPTP: run one subclock per gPTP domain and switch the active
//   one according to best-master selection or policy.
//
// @see ~MultiDomainGptp, ~inet.clock.contract.IClock
//
module MultiClock extends Module like IClock
{
    parameters:
        int numClocks;                                // Number of subclocks
        int activeClockIndex @mutable = default(0);   // Currently active subclock (0..numClocks-1)
        @display("i=block/timer");
        @class(MultiClock);
        @signal[timeChanged](type=simtime_t);
        @statistic[timeChanged](title="Clock time"; source=localSignal(timeChanged); record=vector; interpolationmode=linear; unit=s);
    submodules:
        clock[numClocks]: <default("SettableClock")> like IClock {
            @display("p=200,200,row,200");
        }
}

