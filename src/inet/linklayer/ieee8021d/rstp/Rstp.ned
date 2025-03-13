//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee8021d.rstp;

import inet.linklayer.contract.ISpanningTree;

//
// Implements the Rapid Spanning Tree Protocol (RSTP) as defined in IEEE 802.1D-2004.
// RSTP is an evolution of the original Spanning Tree Protocol (STP) that provides
// significantly faster convergence when topology changes occur in Ethernet networks.
//
// The protocol creates a loop-free logical topology in networks with redundant links by:
// - Electing a single root bridge based on bridge priority and MAC address
// - Calculating the shortest path from each bridge to the root bridge
// - Blocking redundant ports that would create loops
// - Rapidly transitioning ports to forwarding state through proposal/agreement handshakes
// - Quickly responding to topology changes with direct notifications
//
// Key improvements over STP:
// - Faster convergence: typically 1-3 seconds vs. 30-50 seconds in STP
// - Rapid transition to forwarding through proposal/agreement mechanism
// - Backup/alternate port roles for faster failover
// - Direct topology change notification without using the root bridge as relay
// - Edge port concept for immediate transition to forwarding
// - Point-to-point link detection for faster handshakes
//
// This implementation supports all core RSTP features but does not include the
// fallback mechanism to legacy STP when communicating with older bridges that
// don't support RSTP. The module exchanges BPDU messages with other bridges and
// maintains port states in the ~InterfaceTable.
//
// The visualization option allows observing the spanning tree topology in the
// network graphics, with port roles and states clearly indicated.
//
// @see ~Stp, ~Ieee8021dRelay, ~L2NetworkConfigurator, ~InterfaceTable
//
simple Rstp like ISpanningTree
{
    parameters:
        // The hello time is the time between each bridge protocol data unit (BPDU) that is sent on a port.
        // This time is equal to 2 seconds (sec) by default, but you can tune the time to be between 1 and 10 sec.
        double helloTime @unit(s) = default(2s);

        // The forward delay is the time that is spent in the listening and learning state. This time is equal to 6 sec by default,
        // but you can tune the time to be between 4 and 30 sec.
        double forwardDelay @unit(s) = default(6s);

        // The max age timer controls the maximum length of time that passes before a bridge port saves its configuration BPDU information.
        // This time is 20 sec by default, but you can tune the time to be between 6 and 40 sec.
        double maxAge @unit(s) = default(20s);

        // The migrate time is the time after which a non-assigned port becomes designated. This time is equal to 6 sec by default,
        // but you can tune the time to be between 4 and 30 sec.
        double migrateTime @unit(s)= default(3s);

        // The topology change while time is the time while a port will send TC=true (Topology Change Notification)
        double tcWhileTime @unit(s) = default(6s);

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

        // If true, edge ports immediately become designated/forwarding, else they will have to wait to get designated.
        bool autoEdge = default(true);

        @display("i=block/network2");
    gates:
        input relayIn;
        output relayOut;
}
