//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.bgpv4;

import inet.common.SimpleModule;
import inet.routing.contract.IBgp;

//
// Implements the BGP Version 4 routing protocol (RFC 4271).
//
// A single shared XML file (the `bgpConfig` parameter) describes the BGP configuration of
// every router in the network; each ~Bgp module reads the same file and picks out the parts
// that apply to it (the local router is recognized by its interface addresses). The XML has
// one `<BGPConfig>` root element containing:
//
// - `<TimerParams>` - the BGP timers, in seconds, applied to all sessions:
//   - `<connectRetryTime>` - the ConnectRetryTimer interval (RFC 4271).
//   - `<holdTime>` - the Hold Time proposed in the OPEN message.
//   - `<keepAliveTime>` - the KeepAlive interval (typically holdTime/3).
//   - `<startDelay>` - base delay before a router starts its sessions (staggers startup).
//
// - `<AS>` - an autonomous system.
//   - `@id` - the AS number.
//   - `<Router>` - a BGP speaker in this AS, identified by one of its own addresses:
//     - `@interAddr` - an address of this router (recognizes the local router and is its internal address).
//     - `@redistributeInternal`, `@redistributeOspf`, `@redistributeRip` - optional; override the module parameters.
//     - `<Network>` - a prefix this router originates into BGP.
//       - `@address` - the network address to advertise (must exist in the routing table).
//     - `<Neighbor>` - per-neighbor settings.
//       - `@address` - the peer address.
//       - `@nextHopSelf`, `@localPreference`, `@connectedCheck`, `@ebgpMultihop` - optional; override the module parameters.
//
// - `<Session>` - an (external) BGP session between two routers.
//   - `@id` - a session identifier (also orders/staggers session startup).
//   - `<Router>` x2 - the two endpoints, each with `@exterAddr` (its peering address).
//
// - `<DenyRoute>` / `<DenyRouteIN>` / `<DenyRouteOUT>` - prefix filters (in/out/both directions),
//   with `@Address` and `@Netmask`.
// - `<DenyAS>` / `<DenyASIN>` / `<DenyASOUT>` - AS-path filters; the element text is the AS number.
//
// IPv6 / MP-BGP (RFC 4760): set `addressFamily = "ipv6"`, point `routingTableModule` at the
// node's IPv6 routing table, and set `routerId` (the 4-octet BGP Identifier of RFC 4271/6286 -
// an IPv6-only router has no IPv4 address to derive it from). Addresses in the XML are then
// IPv6, and IPv6 reachability is carried in the MP_REACH_NLRI / MP_UNREACH_NLRI path attributes.
// One ~Bgp instance serves a single address family; see the `BgpIpv6Basic` example.
//
simple Bgp extends SimpleModule like IBgp
{
    parameters:
        @class(Bgp);
        string interfaceTableModule;
        string routingTableModule;
        string addressFamily @enum("ipv4","ipv6") = default("ipv4"); // address family this BGP instance operates on (one instance serves one family)
        string routerId = default(""); // the 4-octet BGP Identifier (RFC 4271/6286); required for IPv6, where there is no IPv4 router id to derive it from
        string ospfRoutingModule;
        volatile double startupTime @unit(s) = default(0s); // Delay before starting BGP
        double shutdownTime @unit(s) = default(1s); // On graceful stop, delay before destroying BGP state so TCP can flush the session teardown (FIN)
        double stopOperationTimeout @unit(s) = default(2s); // Timeout for the lifecycle stop operation
        xml bgpConfig;
        double ExternalPeerStartDelayOffset @unit(s) = default(0.5s);
        bool redistributeRip = default(false);        // Redistribute routes learned by RIP into BGP
        string redistributeOspf = default("");        // Redistribute routes learned by OSPF into BGP (O IA E1 E2)
        bool redistributeInternal = default(false);   // Redistribute BGP routes into OSPF (as external routes)
        bool nextHopSelf = default(false);
        int localPreference = default(100);
        bool connectedCheck = default(true);
        int ebgpMultihop = default(1);
        @display("i=block/network2");
        @selfMessageKinds(inet::bgp::BgpSelfMsgKind);
    gates:
        input socketIn @messageKinds(inet::TcpStatusInd);
        output socketOut @messageKinds(inet::TcpCommandCode);
}

