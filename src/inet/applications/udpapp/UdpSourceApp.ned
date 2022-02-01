//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

//
// This module implements a UDP application that only sends packets.
//
module UdpSourceApp extends UdpApp
{
    parameters:
        sink.typename = "";
}
