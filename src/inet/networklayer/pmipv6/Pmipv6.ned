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

        @signal[proxyBindingUpdateSent](type=long);              // MAG: emits the requested binding lifetime in seconds (0 = deregistration)
        @signal[proxyBindingAcknowledgementReceived](type=long); // MAG: emits the granted binding lifetime in seconds (0 = deregistration acknowledged)
        @signal[proxyBindingUpdateReceived](type=long);          // LMA: emits the requested binding lifetime in seconds (0 = deregistration)
        @signal[homeNetworkPrefixReanchored](type=long);         // LMA: emits value=1 when a mobile node's home network prefix is re-pointed to another gateway
        @signal[bindingCacheSize](type=long);                    // LMA: emits the number of binding cache entries after a binding is created, refreshed or removed
        @signal[packetDropped](type=inet::Packet);

        @statistic[proxyBindingUpdateSent](title="Proxy Binding Updates sent"; record=count,vector; interpolationmode=none);
        @statistic[proxyBindingAcknowledgementReceived](title="Proxy Binding Acknowledgements received"; record=count,vector; interpolationmode=none);
        @statistic[proxyBindingUpdateReceived](title="Proxy Binding Updates received"; record=count,vector; interpolationmode=none);
        @statistic[homeNetworkPrefixReanchored](title="home network prefix re-anchored to another gateway"; record=count,vector; interpolationmode=none);
        @statistic[bindingCacheSize](title="binding cache entries"; record=vector,max; interpolationmode=sample-hold);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input fromIPv6;
        output toIPv6;
}
