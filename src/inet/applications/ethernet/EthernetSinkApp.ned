//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ethernet;

//
// This module implements an Ethernet application that only receives packets.
//
module EthernetSinkApp extends EthernetApp
{
    parameters:
        source.typename = "";
        io.destPort = -1;
}
