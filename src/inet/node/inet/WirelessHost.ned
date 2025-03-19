//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

//
// A mobile device with wireless connectivity operating in infrastructure mode.
// Extends StandardHost by adding a preconfigured IEEE 802.11 wireless interface, making it suitable
// for simulating devices in WiFi networks.
//
// The WirelessHost is designed to connect to wireless access points and operates in infrastructure mode
// by default. It includes an IEEE 802.11 management module configured as a station (STA), allowing it
// to associate with access points, perform scanning, authentication, and other WiFi management functions.
//
// @see ~AccessPoint
//
module WirelessHost extends StandardHost
{
    parameters:
        numWlanInterfaces = default(1);
        @display("i=device/wifilaptop");
}

