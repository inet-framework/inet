//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This module allows or forbids packets to pass through depending on whether
// the gate is open or closed. The gate is open and closed according to the list
// of change times periodically.
//
simple PeriodicGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        bool initiallyOpen @mutable = default(true); // specifies if the gate is initially open or closed
        double offset @mutable @unit(s) = default(0s); // specifies where the period starts in the list of gate state change durations; where it is in the period at the start
        object durations @mutable @unit(s) = default([]); // list of time durations within the period that determine if the gate is open or closed; period is the sum of the durations
        bool scheduleForAbsoluteTime = default(true); // when a clock is used relative means that setting the clock will not affect the simulation time of the event
        int openSchedulingPriority = default(0); // FES scheduling priority for the next gate open event
        int closeSchedulingPriority = default(0); // FES scheduling priority for the next gate close event
        bool enableImplicitGuardBand = default(true); // implicit guard band means that a packet is not allowed to be forwarded if it cannot finish transmission before the end of window.
        @class(PeriodicGate);
        @signal[guardBandStateChanged](type=bool);
        @statistic[guardBandState](title="guard band state"; type=enum; enum=ON, OFF; source=guardBandStateChanged; record=vector; interpolationmode=sample-hold);
}
