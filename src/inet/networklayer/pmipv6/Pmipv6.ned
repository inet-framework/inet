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

        // MAG: which of this gateway's interfaces are access links, as a name pattern. This is
        // configuration and not a consequence of who is attached: RFC 5213 Section 6.10.5's
        // check on where a packet came from has to hold before the first mobile node
        // registers and after the last one leaves, not only in between.
        string accessInterfaces = default("wlan*");

        // MAG: mobile-node policy profiles, one per access interface, e.g.
        //   <mobileNodes>
        //     <mobileNode accessInterface="wlan0" id="mn1@example.com"
        //                 homeNetworkPrefix="2001:db8:1::" prefixLength="64"/>
        //   </mobileNodes>
        // (accessInterface may be omitted to match any access interface)
        xml mobileNodeProfiles = default(xml("<mobileNodes/>"));

        // MAG: treat a link-layer transmission failure toward a mobile node as evidence that it
        // has left the access link. RFC 5213 leaves the detection method to the access technology;
        // on IEEE 802.11 this is the access point giving up on a frame after its retry limit.
        bool detectTransmissionFailure = default(true);

        // MAG: when nothing has been seen from a mobile node for this long, ask it whether it is
        // still there rather than assume either way -- a Neighbour Solicitation to its link-local
        // address, which RFC 5213 Section 6.13 names as one of the ways a gateway may detect that
        // a node has gone. A node that is present but idle answers and stays registered. 0
        // disables probing, leaving detection to the link-layer events above.
        double presenceProbeDelay @unit(s) = default(5s);
        // How long the gateway then waits for the answer is NOT configured here: Neighbour
        // Discovery is asked how long its own exchange can take, because that depends on
        // constants of the access link -- notably DelayFirstProbeTime, which is five seconds
        // and elapses before a solicitation is sent at all. A number chosen here would have
        // to track those, and a wrong one declares a present node absent.
        double presenceCheckInterval @unit(s) = default(1s); // MAG: how often presenceProbeDelay is evaluated

        // LMA: how long a binding cache entry survives a deregistration, so that a mobile node
        // that reappears at another gateway keeps its mobility session (RFC 5213 Section 9.1
        // names this MinDelayBeforeBCEDelete and gives it 10000 ms)
        double minDelayBeforeBindingCacheEntryDelete @unit(s) = default(10s);

        // MAG: retransmission of an unanswered Proxy Binding Update. RFC 5213 Section 6.9.4
        // takes these from Mobile IPv6 (RFC 6275 Section 12), where INITIAL_BINDACK_TIMEOUT is
        // 1 s and MAX_BINDACK_TIMEOUT is 32 s, and doubles the interval after each attempt.
        double initialBindingAckTimeout @unit(s) = default(1s);
        double maxBindingAckTimeout @unit(s) = default(32s);

        // Order Proxy Binding Updates by the timestamp the gateway puts in them, so that one
        // delayed in the network cannot undo a later registration. RFC 5213 Section 9.3 calls
        // this TimestampBasedApproachInUse and enables it by default. With it off, the messages
        // carry no timestamp and the anchor falls back to the per-session sequence number,
        // which can only order messages from the gateway currently serving the node: RFC 5213
        // Section 5.5 leaves it out of scope how a gateway the node has just moved to would
        // learn the sequence number the previous one had reached.
        bool timestampBasedOrdering = default(true);

        double bindingLifetime @unit(s) = default(3600s);                    // MAG: lifetime requested in Proxy Binding Updates
        double maxBindingLifetime @unit(s) = default(3600s);                 // LMA: the longest lifetime it grants; a shorter request is granted as asked
        double bindingRefreshRatio = default(0.8);                           // MAG: re-register after this fraction of the granted lifetime; 0 disables refreshing
        double homeNetworkPrefixAdvValidLifetime @unit(s) = default(3600s);  // MAG: valid lifetime of the advertised home network prefix
        double homeNetworkPrefixAdvPreferredLifetime @unit(s) = default(1800s); // MAG: preferred lifetime of the advertised home network prefix
        @display("i=block/network2");

        @signal[proxyBindingUpdateSent](type=long);              // MAG: emits the requested binding lifetime in seconds (0 = deregistration)
        @signal[proxyBindingAcknowledgementReceived](type=long); // MAG: emits the granted binding lifetime in seconds (0 = deregistration acknowledged)
        @signal[proxyBindingUpdateReceived](type=long);          // LMA: emits the requested binding lifetime in seconds (0 = deregistration)
        @signal[homeNetworkPrefixReanchored](type=long);         // LMA: emits value=1 when a mobile node's home network prefix is re-pointed to another gateway
        @signal[mobileNodeDetached](type=long);                   // MAG: emits value=1 when a mobile node is found to have left the access link
        @signal[bindingCacheSize](type=long);                    // LMA: emits the number of binding cache entries after a binding is created, refreshed or removed
        @signal[packetDropped](type=inet::Packet);

        @statistic[proxyBindingUpdateSent](title="Proxy Binding Updates sent"; record=count,vector; interpolationmode=none);
        @statistic[proxyBindingAcknowledgementReceived](title="Proxy Binding Acknowledgements received"; record=count,vector; interpolationmode=none);
        @statistic[proxyBindingUpdateReceived](title="Proxy Binding Updates received"; record=count,vector; interpolationmode=none);
        @statistic[homeNetworkPrefixReanchored](title="home network prefix re-anchored to another gateway"; record=count,vector; interpolationmode=none);
        @statistic[mobileNodeDetached](title="mobile node detachments detected"; record=count,vector; interpolationmode=none);
        @statistic[bindingCacheSize](title="binding cache entries"; record=vector,max; interpolationmode=sample-hold);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input fromIPv6;
        output toIPv6;
}
