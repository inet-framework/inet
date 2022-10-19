//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv4;

//
// Stores the network address translation table.
//
simple Ipv4NatTable
{
    parameters:
        string networkProtocolModule = default("^.ip");
        xml config = default(xml("<config></config>")); // XML configuration parameters for network address translation
        @display("i=block/table");
}
