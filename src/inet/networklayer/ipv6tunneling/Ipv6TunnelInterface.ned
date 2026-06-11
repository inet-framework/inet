//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv6tunneling;

import inet.linklayer.contract.ITunnelInterface;
import inet.networklayer.common.NetworkInterface;

//
// A virtual network interface that tunnels IPv6 traffic in IPv6 (RFC 2473).
// Traffic routed to this interface is encapsulated toward `destination` and
// handed back to IPv6 for forwarding; the IPv6 core needs no tunneling-specific
// knowledge. Plugs into the `tun[]` slot of nodes like any ITunnelInterface.
//
module Ipv6TunnelInterface extends NetworkInterface like ITunnelInterface
{
    parameters:
        string interfaceTableModule;
        string source;      // tunnel entry point (an address of this node)
        string destination; // tunnel exit point
        int mtu @unit(B) = default(1500B);
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        tunnel: Ipv6Tunnel {
            source = parent.source;
            destination = parent.destination;
            mtu = parent.mtu;
        }
    connections:
        upperLayerIn --> tunnel.upperLayerIn;
        tunnel.upperLayerOut --> upperLayerOut;
}
