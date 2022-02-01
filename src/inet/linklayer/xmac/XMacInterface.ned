//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.xmac;

import inet.linklayer.common.WirelessInterface;

//
// This module implements a wireless network interface using the X-MAC protocol.
//
module XMacInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskScalarRadio");
        mac.typename = "XMac";
        queue.typename = default("DropTailQueue");
        queue.packetCapacity = default(20);
}

