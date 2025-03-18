//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.common.SimpleModule;

//
// Maintains a table of active Ethernet sockets. Stores socket descriptors and
// their associated parameters such as protocol-specific filtering criteria.
// Provides lookup functionality for the EthernetSocketCommandProcessor and
// EthernetSocketPacketProcessor to map between socket IDs and their
// configurations. Essential component of the socket-based interface to Ethernet
// services.
//
simple EthernetSocketTable extends SimpleModule
{
    parameters:
        @class(EthernetSocketTable);
        @display("i=block/table");
}
