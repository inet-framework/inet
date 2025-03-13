//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.shortcut;

import inet.linklayer.common.WirelessInterface;

//
// Provides a simplified network interface that creates direct shortcuts between
// nodes at the MAC layer, bypassing the physical layer entirely. This interface
// extends the WirelessInterface but deliberately omits the radio component and
// uses a minimal queue implementation.
//
// The interface uses ShortcutMac to establish direct connections between peer
// nodes, simulating transmission delays and packet losses without the computational
// overhead of detailed physical layer simulation. This approach significantly
// improves simulation performance while maintaining essential timing characteristics.
//
// Particularly useful for:
// - Large-scale network simulations where physical layer details are less important
// - Rapid prototyping and testing of higher-layer protocols
// - Performance benchmarking with controlled link characteristics
//
// @see ~ShortcutMac, ~WirelessInterface
//
module ShortcutInterface extends WirelessInterface
{
    parameters:
        radio.typename = "";
        mac.typename = "ShortcutMac";
        queue.typename = "OmittedPacketQueue";
}
