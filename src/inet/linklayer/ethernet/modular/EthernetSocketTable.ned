//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

//
// Maintains a table of Ethernet sockets that allow applications to send and
// receive Ethernet frames directly. Each socket entry contains information
// about the socket's identifier, protocol, local and remote addresses, and
// other socket-specific parameters.
//
// The socket table is used by EthernetSocketCommandProcessor to handle socket
// commands from applications (open, close, bind, etc.) and by EthernetSocketPacketProcessor
// to deliver incoming packets to the appropriate application. This enables
// direct Ethernet communication without the need for higher layer protocols.
//
// @see ~EthernetSocketCommandProcessor, ~EthernetSocketPacketProcessor, ~EthernetLayer
//
simple EthernetSocketTable
{
    parameters:
        @display("i=block/table");
}
