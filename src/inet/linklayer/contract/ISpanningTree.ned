//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for Spanning Tree protocols
//
moduleinterface ISpanningTree
{
    parameters:
        @display("i=block/network2");
        double helloTime @unit(s);
        double forwardDelay @unit(s);
        double maxAge @unit(s);
        int bridgePriority;
        bool visualize;
    gates:
        input relayIn;
        output relayOut;
}

