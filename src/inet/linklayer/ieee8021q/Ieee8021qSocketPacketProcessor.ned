//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

//
// Processes packets related to IEEE 802.1Q VLAN sockets. Delivers packets to
// registered sockets based on protocol and VLAN ID matching. Duplicates packets
// for delivery to interested sockets and supports "stealing" packets to prevent
// them from being forwarded. Enables applications to communicate directly with
// specific VLANs and implements protocol handlers for VLAN-specific traffic.
// Works in conjunction with ~Ieee8021qSocketTable to provide a socket-based API
// for VLAN communication.
//
// @see Ieee8021qSocketCommandProcessor
//
simple Ieee8021qSocketPacketProcessor extends PacketPusherBase like IPacketFlow
{
    parameters:
        @class(Ieee8021qSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}

