//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ethernet;

//
// This module implements an Ethernet application that only sends packets.
//
module EthernetSourceApp extends EthernetApp
{
    parameters:
        sink.typename = "";
}
