//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

import inet.common.SimpleModule;

//
// Maintains a registry of IEEE 802.2 LLC sockets. Stores socket information
// including socket IDs and Service Access Point (SAP) values. Provides methods
// to add, remove, and find sockets based on local and remote SAP values.
// Works in conjunction with socket command and packet processors to enable
// socket-based access to LLC services for applications.
//
simple Ieee8022LlcSocketTable extends SimpleModule
{
    parameters:
        @class(Ieee8022LlcSocketTable);
        @display("i=block/table");
}

