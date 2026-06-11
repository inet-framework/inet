//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv6tunneling;

import inet.common.SimpleModule;

//
// Worker module of ~Ipv6TunnelInterface: performs IPv6-in-IPv6 (RFC 2473)
// encapsulation. A packet routed to this interface (from the IPv6 layer) is
// wrapped in an outer IPv6 header and handed back to IPv6, which forwards the
// encapsulated datagram toward the tunnel exit.
//
simple Ipv6Tunnel extends SimpleModule
{
    parameters:
        @class(Ipv6Tunnel);
        string interfaceTableModule;
        string source;      // tunnel entry point (an address of this node)
        string destination; // tunnel exit point
        int mtu @unit(B) = default(1500B);
        @display("i=block/tunnel");
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
    gates:
        input upperLayerIn;
        output upperLayerOut;
}
