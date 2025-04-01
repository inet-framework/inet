//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

//
// Configures multiple `PeriodicGate` modules according to a gate control list.
// The modules are looked for at the `^.transmissionGate[*]` relative module path.
//
simple GateControlList
{
    parameters:
        object durations @unit(s) = default([]); // List of entry durations, e.g. [4ms, 2ms, 2ms, 2ms]
        object gateStates = default([]); // List of gate states, e.g. ["10000000", "01111111", "10000000", "01111111"]
        int numGates = default(8); // Number of configured PeriodicGate modules
        string mapping; // String specifying the gate index for the ith gate state, e.g. "76543210" means reverse gate index order
        @class(GateControlList);
        @display("i=block/table");
}
