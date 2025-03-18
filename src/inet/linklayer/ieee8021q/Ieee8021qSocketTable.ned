//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

//
// Maintains a table of IEEE 802.1Q VLAN sockets. Each socket entry contains a
// socket ID, protocol, VLAN ID, and a flag indicating whether to steal packets.
// The module provides functionality to add, remove, and find sockets based on
// protocol and VLAN ID criteria. It enables applications to communicate through
// specific VLANs by associating sockets with VLAN tags, facilitating the
// routing of packets to the appropriate protocol handlers.
//
simple Ieee8021qSocketTable
{
    parameters:
        @display("i=block/table");
}
