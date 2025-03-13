//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.shortcut;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.ILinkLayer;
import inet.linklayer.contract.IMacProtocol;

//
// Implements a simplified MAC protocol that creates direct shortcuts between
// peer MAC modules, completely bypassing the physical layer for more efficient
// simulation. This module directly sends packets to the destination MAC without
// any physical layer processing, while still simulating key physical layer
// characteristics.
//
// Physical layer effects are modeled through configurable parameters:
// - lengthOverhead: Simulates header/trailer bits added by the physical layer
// - durationOverhead: Accounts for additional transmission time (preambles, etc.)
// - propagationDelay: Simulates signal propagation time between nodes
// - packetLoss: Allows for random packet loss with configurable probability
//
// This module is particularly useful for simulations where detailed physical
// layer modeling is not required, significantly improving simulation performance
// while maintaining essential timing and loss characteristics.
//
// @see ~ShortcutInterface
//
simple ShortcutMac extends MacProtocolBase like ILinkLayer, IMacProtocol
{
    parameters:
        string address @mutable = default("auto");   // MAC address as a hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        double bitrate @unit(bps);
        int mtu @unit(B) = default(1500B);
        volatile int lengthOverhead @unit(b) = default(0b); // Extra length added to packets
        volatile double durationOverhead @unit(s) = default(0s); // Extra transmission duration added to packets
        volatile double propagationDelay @unit(s) = default(0s); // Propagation time to all destinations
        volatile double packetLoss = default(0); // Packet loss probability
        @class(ShortcutMac);
        @display("i=block/rxtx");
    gates:
        input peerIn;
}
