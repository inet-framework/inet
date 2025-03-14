//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.tester;

//
// Diagnostic module for analyzing Spanning Tree Protocol (STP) network configurations.
// Extracts the network topology considering only enabled links (state=FORWARDING),
// then performs graph analysis to verify if the resulting topology is loop-free,
// fully connected, and forms a proper tree structure. Uses a modified depth-first
// search algorithm with cycle detection to analyze the network graph.
//
// @see Stp, Rstp, Ieee8021dRelay
//
simple StpTester
{
    parameters:
        double checkTime @unit(s) = default(500ms);

        @display("i=block/cogwheel");
}
