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
// This module implements a simple shortcut to peer MAC protocol that completely
// bypasses the physical layer. Packets received from the upper layer protocols
// are never lost. The MAC protocol directly sends packets to the destination
// MAC protocol without any physical layer processing. Physical layer overhead
// is simply simulated by overhead bits, overhead transmission duration and
// a propagation delay.
//
simple ShortcutMac extends MacProtocolBase like ILinkLayer, IMacProtocol
{
    parameters:
        string address @mutable = default("auto");   // MAC address as hex string (12 hex digits), or
                                                     // "auto". "auto" values will be replaced by
                                                     // a generated MAC address in init stage 0.
        double bitrate @unit(bps);
        int mtu @unit(B) = default(1500B);
        volatile int lengthOverhead @unit(b) = default(0b); // extra length added to packets
        volatile double durationOverhead @unit(s) = default(0s); // extra transmission duration added to packets
        volatile double propagationDelay @unit(s) = default(0s); // propagation time to all destinations
        volatile double packetLoss = default(0); // packet loss probability
        @class(ShortcutMac);
        @display("i=block/rxtx");
    gates:
        input peerIn;
}

