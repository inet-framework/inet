//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.tester;

import inet.common.SimpleModule;

//
// Network tester module.
// Shows if a network topology is loop-free, all nodes are connected, or it has a tree topology.
//
simple StpTester extends SimpleModule
{
    parameters:
        @class(StpTester);
        double checkTime @unit(s) = default(500ms);

        @display("i=block/cogwheel");
}

