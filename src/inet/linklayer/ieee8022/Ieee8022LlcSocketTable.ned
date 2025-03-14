//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Registry for IEEE 802.2 LLC sockets that maintains socket information for Service Access Point (SAP) 
// based communication. Stores mappings between socket IDs and their associated local and remote SAP values. 
// Provides lookup functionality to find sockets matching specific SAP combinations, enabling the socket 
// packet processor to deliver LLC frames to the appropriate applications based on DSAP/SSAP values.
//
// @see ~Ieee8022LlcSocketCommandProcessor, ~Ieee8022LlcSocketPacketProcessor
//
simple Ieee8022LlcSocketTable
{
    parameters:
        @display("i=block/table");
}
