//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Defines the interface for Spanning Tree Protocol implementations that prevent
// bridge loops in Ethernet networks. This interface specifies the common parameters
// and gates required by any spanning tree protocol implementation.
//
// Spanning Tree Protocols are essential in networks with redundant links, as they:
// - Prevent broadcast storms and other undesirable effects of bridge loops
// - Automatically establish a loop-free active topology
// - Provide redundancy by dynamically reconfiguring the network when links fail
// - Maintain connectivity between all nodes in the network
//
// The key parameters defined in this interface control the protocol's timing behavior:
// - helloTime: Controls how often configuration messages are sent
// - forwardDelay: Determines how quickly ports transition between states
// - maxAge: Sets the maximum time configuration information is retained
// - bridgePriority: Influences which bridge becomes the root of the spanning tree
//
// Modules implementing this interface include ~Stp (the original Spanning Tree Protocol)
// and ~Rstp (Rapid Spanning Tree Protocol), which provides faster convergence.
//
// @see ~Stp, ~Rstp, ~Ieee8021dRelay
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
