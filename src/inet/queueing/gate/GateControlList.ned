//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

//
// This module configures multiple PeriodicGate modules according to a gate control list.
//
simple GateControlList
{
    parameters:
        int numGates = default(8);
        object durations @mutable @unit(s) = default([]); // list of entry durations, e.g. [4ms, 2ms, 2ms, 2ms]
        object gateStates @mutable = default([]); // list of gateStates, e.g. ["10000000", "01111111", "10000000", "01111111"]
        @class(GateControlList);
        @display("i=block/table");
}
