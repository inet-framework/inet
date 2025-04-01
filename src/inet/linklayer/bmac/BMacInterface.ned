//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.bmac;

import inet.linklayer.common.WirelessInterface;

//
// Implements a wireless network interface using the B-MAC protocol.
//
module BMacInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskRadio");
        mac.typename = "BMac";
        queue.typename = default("DropTailQueue");
        queue.packetCapacity = default(20);
}

