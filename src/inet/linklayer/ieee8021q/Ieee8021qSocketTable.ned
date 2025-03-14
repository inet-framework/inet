//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

//
// Registry for IEEE 802.1Q sockets that maintains socket information for protocol-based communication. 
// Stores mappings between socket IDs and their associated protocols, VLAN IDs, and packet stealing flags. 
// Provides lookup functionality to find sockets matching specific protocol and VLAN ID combinations, 
// enabling the socket packet processor to deliver packets to the appropriate applications.
//
// @see ~Ieee8021qSocketCommandProcessor, ~Ieee8021qSocketPacketProcessor
//
simple Ieee8021qSocketTable
{
    parameters:
        @display("i=block/table");
}
