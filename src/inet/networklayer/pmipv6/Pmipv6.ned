//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.pmipv6;

import inet.common.SimpleModule;

//
// Implements Proxy Mobile IPv6 (RFC 5213): network-based mobility management.
// Unlike Mobile IPv6 (~Mipv6), the mobile node runs no mobility software; the
// network handles mobility through two entities, both implemented by this module
// and selected by the `isLocalMobilityAnchor` / `isMobileAccessGateway` parameters:
//
//  - the Local Mobility Anchor (LMA), the home-prefix anchor and tunnel endpoint,
//    analogous to a Mobile IPv6 home agent; and
//  - the Mobile Access Gateway (MAG), the access router that detects mobile-node
//    attachment, sends Proxy Binding Updates to the LMA on the mobile node's behalf,
//    sets up the tunnel to the LMA, and advertises the mobile node's home network
//    prefix on the access link.
//
// Proxy Binding Updates are ordered by timestamp (RFC 5213 Section 5.5): the MAG
// puts its clock (simulation time) into the Timestamp option, and the LMA refuses
// a Proxy Binding Update that is not newer than the last one it accepted for the
// mobile node. The sequence-number scheme that RFC 5213 offers instead is not
// implemented: it needs a way for a MAG to learn the sequence number the previous
// MAG sent, which RFC 5213 leaves out of scope.
//
// It is a submodule of ~Ipv6NetworkLayer (enabled by its `hasPmipv6` parameter).
//
// @see ~LocalMobilityAnchor, ~MobileAccessGateway
//
simple Pmipv6 extends SimpleModule
{
    parameters:
        @class(Pmipv6);
        string interfaceTableModule;          // path to the InterfaceTable module
        string ipv6NeighbourDiscoveryModule;  // path to the Ipv6NeighbourDiscovery module
                                              // (the IPv6 routing table is resolved automatically)
        bool isLocalMobilityAnchor = default(false);
        bool isMobileAccessGateway = default(false);

        // MAG: the address of the LMA to register mobile nodes with
        string localMobilityAnchorAddress = default("");

        // MAG: mobile-node policy profiles, one per access interface, e.g.
        //   <mobileNodes>
        //     <mobileNode accessInterface="wlan0" id="mn1@example.com"
        //                 homeNetworkPrefix="2001:db8:1::" prefixLength="64"/>
        //   </mobileNodes>
        // (accessInterface may be omitted to match any access interface)
        xml mobileNodeProfiles = default(xml("<mobileNodes/>"));

        double bindingLifetime @unit(s) = default(3600s);                    // lifetime requested in Proxy Binding Updates
        double homeNetworkPrefixAdvValidLifetime @unit(s) = default(3600s);  // MAG: valid lifetime of the advertised home network prefix
        double homeNetworkPrefixAdvPreferredLifetime @unit(s) = default(1800s); // MAG: preferred lifetime of the advertised home network prefix
        @display("i=block/network2");
        @signal[packetDropped](type=cPacket);
    gates:
        input fromIPv6;
        output toIPv6;
}
