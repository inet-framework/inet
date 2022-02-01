//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.tester;

//
// Network tester module.
// Shows if a network topology is loopfree, all nodes are connected or it has tree topology.
//
simple StpTester
{
    parameters:
        double checkTime @unit(s) = default(500ms);

        @display("i=block/cogwheel");
}

