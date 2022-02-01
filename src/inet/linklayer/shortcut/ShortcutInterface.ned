//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.shortcut;

import inet.linklayer.common.WirelessInterface;

//
// This module implements a simpistic network interface that uses a shortcut to
// the receiver at the MAC layer.
//
module ShortcutInterface extends WirelessInterface
{
    parameters:
        radio.typename = "";
        mac.typename = "ShortcutMac";
        queue.typename = "OmittedPacketQueue";
}

