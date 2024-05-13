//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

//
// This module configures multiple PeriodicGate modules according to a gate control list.
// The modules are looked for at ^.transmissionGate[*] relative module path.
//
simple GateControlList
{
    parameters:
        object durations @mutable @unit(s) = default([]); // List of entry durations, e.g. [4ms, 2ms, 2ms, 2ms]
        object gateStates @mutable = default([]); // List of gateStates, e.g. ["10000000", "01111111", "10000000", "01111111"]
        int numGates = default(8); // Number of configured PeriodicGate modules
        string mapping; // String specifying the gate index for the ith gate states, e.g. "76543210" means reverse gate index order
        @class(GateControlList);
        @display("i=block/table");
}
