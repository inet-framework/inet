//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.stp;

import inet.linklayer.contract.ISpanningTree;

//
// The Spanning Tree Protocol (STP) is a network protocol that ensures a loop-free
// topology for any bridged Ethernet local area network. The basic function of STP
// is to prevent bridge loops and the broadcast radiation that results from them.
// Spanning tree also allows a network design to include spare (redundant) links to
// provide automatic backup paths if an active link fails, without the danger of
// bridge loops, or the need for manual enabling/disabling of these backup links.
//
// There are various versions of Spanning Tree algorithm and protocol. This module
// implements it according to the IEEE 802.1D-1998 specification.
//
// STP makes use of per-port configuration data such as cost and priority.
// They can be set using ~L2NetworkConfigurator. Port state is kept in the switch's
// ~InterfaceTable.
//
// @see ~L2NetworkConfigurator, ~InterfaceTable
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

