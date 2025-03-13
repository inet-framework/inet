//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.stp;

import inet.linklayer.contract.ISpanningTree;

//
// Implements the original Spanning Tree Protocol (STP) as defined in IEEE 802.1D-1998.
// STP creates a loop-free logical topology in Ethernet networks with redundant links,
// preventing broadcast storms while maintaining network resilience.
//
// The protocol operates by:
// - Electing a single root bridge based on bridge priority and MAC address
// - Calculating the shortest path from each bridge to the root bridge
// - Blocking redundant ports that would create loops
// - Transitioning ports through multiple states (blocking, listening, learning, forwarding)
// - Reconfiguring the network topology when links fail or bridges are added/removed
//
// Key features:
// - Automatic root bridge election through Bridge Protocol Data Units (BPDUs)
// - Path cost calculation based on link speeds
// - Port state management (blocking, listening, learning, forwarding)
// - Topology change detection and propagation
// - Relatively slow convergence (typically 30-50 seconds)
//
// The module exchanges BPDU messages with other bridges to determine the network
// topology. It maintains port states in the ~InterfaceTable and can be configured
// using the ~L2NetworkConfigurator. The visualization option allows observing the
// spanning tree topology in the network graphics.
//
// This implementation is now considered legacy, as it has been superseded by
// the Rapid Spanning Tree Protocol (~Rstp) which provides faster convergence.
//
// @see ~Rstp, ~Ieee8021dRelay, ~L2NetworkConfigurator, ~InterfaceTable
//
simple Stp like ISpanningTree
{
    parameters:

        // The hello time is the time between each bridge protocol data unit (BPDU) that is sent on a port.
        // This time is equal to 2 seconds (sec) by default, but you can tune the time to be between 1 and 10 sec.
        double helloTime @unit(s) = default(2s);

        // The forward delay is the time that is spent in the listening and learning state. This time is equal to 15 sec by default,
        // but you can tune the time to be between 4 and 30 sec.
        double forwardDelay @unit(s) = default(15s);

        // The max age timer controls the maximum length of time that passes before a bridge port saves its configuration BPDU information.
        // This time is 20 sec by default, but you can tune the time to be between 6 and 40 sec.
        double maxAge @unit(s) = default(20s);

        // The bridge priority determines which bridge is elected as the root bridge. If the switch has a bridge priority that is lower than all
        // the other switches, the other switches automatically select the switch as the root switch.
        int bridgePriority = default(32768);

        // The path to the InterfaceTable module
        string interfaceTableModule;

        // The path to the MacForwardingTable module
        string macTableModule;

        // Shows the spanning tree by coloring connections in the network graphics.
        // Label ethernet interface with port role and status. Mark root switch
        bool visualize = default(false);

        @display("i=block/network2");
    gates:
        input relayIn;
        output relayOut;
}
