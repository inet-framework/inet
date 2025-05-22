//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Processes IEEE 802.1Q socket commands from applications and manages socket
// registrations in the socket table. Handles bind, close, and destroy commands
// for IEEE 802.1Q sockets, enabling applications to send and receive VLAN-tagged
// frames. Supports filtering traffic based on protocol and VLAN ID, providing a
// socket-based interface to the IEEE 802.1Q layer. Works in conjunction with
// ~Ieee8021qSocketTable to maintain socket state and ~Ieee8021qSocketPacketProcessor
// to deliver packets to the appropriate applications.
//
simple Ieee8021qSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8021qSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}

