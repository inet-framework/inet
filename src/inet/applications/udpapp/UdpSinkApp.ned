//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

//
// This module implements a UDP application that only receives packets.
//
module UdpSinkApp extends UdpApp
{
    parameters:
        source.typename = "";
        io.destPort = -1;
}
