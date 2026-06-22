//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv6;

import inet.common.SimpleModule;
import inet.networklayer.contract.IRoutingTable;

//
// IPv6 Routing Table and Neighbor Discovery data structures.
// NOTE: This component MUST be named as `routingTable6` inside
// a ~StandardHost/~Router etc. in order to be accessible by the
// ~Ipv6 and other modules.
//
// <b>XML Configuration (legacy)</b>
//
// NOTE: For new simulations, prefer ~Ipv6NetworkConfigurator which provides
// automatic prefix assignment, hierarchical addressing, route computation,
// and per-interface RA/NDP parameter configuration using pattern-based XML.
//
// The `routes` XML parameter allows manual configuration of interface
// IPv6 settings and IPv6 tunnels. The root element is `<routingTable>`,
// containing `<local>` elements that target specific nodes. Only `<local>`
// elements whose `node` attribute matches the host module's full name
// (i.e. `getFullName()`) are processed.
//
// Each `<local>` element may contain `<interface>` and `<tunnel>` child elements.
//
// <b>Interface configuration</b>
//
// The `<interface>` element configures an IPv6 interface. Attributes:
//  - `name` (required): the interface name (e.g. "eth0", "wlan0")
//  - `AdvSendAdvertisements` (required): enable/disable Router Advertisement
//    sending ("on"/"true"/"yes" or "off"/"false"/"no")
//
// The `<interface>` element may contain the following child elements:
//
//  - `<AdvPrefix>`: Defines an advertised prefix. The element text is the
//    prefix in "address/length" notation (e.g. "2001:db8:1::/64"). Attributes:
//     - `AdvValidLifetime` (required): valid lifetime in seconds
//     - `AdvPreferredLifetime` (required): preferred lifetime in seconds
//     - `AdvOnLinkFlag` (required): whether the prefix is on-link ("true"/"false")
//     - `AdvAutonomousFlag` (required): whether the prefix can be used for SLAAC ("true"/"false")
//
//  - `<inetAddr>`: Assigns a static IPv6 address to the interface. The element
//    text is the IPv6 address. Attributes:
//     - `tentative` (required): whether the address is tentative ("true"/"false")
//
// <b>Tunnel configuration</b>
//
// The `<tunnel>` element contains `<tunnelEntry>` child elements. Each
// `<tunnelEntry>` defines an IPv6-in-IPv6 tunnel. Attributes:
//  - `entryPoint` (required): tunnel entry point IPv6 address
//  - `exitPoint` (required): tunnel exit point IPv6 address
//
// Each `<tunnelEntry>` must contain exactly one `<triggers>` child element:
//  - `destination` (required): the destination prefix that triggers tunneling
//
// <b>Example</b>
//
// <pre>
// <routingTable>
//   <local node="router1">
//     <interface name="eth0" AdvSendAdvertisements="on">
//       <AdvPrefix AdvValidLifetime="2592000" AdvOnLinkFlag="true"
//                  AdvPreferredLifetime="604800" AdvAutonomousFlag="true">
//         2001:db8:1::/64
//       </AdvPrefix>
//       <inetAddr tentative="false">2001:db8:1::1</inetAddr>
//     </interface>
//     <tunnel>
//       <tunnelEntry entryPoint="2001:db8::1" exitPoint="2001:db8::2">
//         <triggers destination="2001:db8:3::"/>
//       </tunnelEntry>
//     </tunnel>
//   </local>
// </routingTable>
// </pre>
//
// @see ~Ipv6, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
simple Ipv6RoutingTable extends SimpleModule like IRoutingTable
{
    parameters:
        @class(Ipv6RoutingTable);
        string interfaceTableModule;   // The path to the InterfaceTable module
        displayStringTextFormat = default("{numRoutes} routes\n{numDestCache} destcache entries");
        @display("i=block/table");
        xml routes = default(xml("<routingTable/>"));
        bool forwarding = default(false);
        bool isRouter = default(this.forwarding);
        bool multicastForwarding = default(false);
        bool useAdminDist = default(false);     // Use Cisco-like administrative distances
        // Router Advertisement defaults (applied to all router interfaces)
        bool advManagedFlag = default(false); // "Managed address configuration" (M) flag in RAs
        bool advOtherConfigFlag = default(false); // "Other configuration" (O) flag in RAs
        int advLinkMtu = default(1280); // MTU option in RAs (1280 = IPv6 minimum MTU, 0 = don't include)
        int advCurHopLimit = default(30); // Cur Hop Limit field in RAs (0 = unspecified)
        double advDefaultLifetime @unit(s) = default(-1s); // Router Lifetime in RAs (-1 = auto: 3*maxRtrAdvInterval)
        @signal[routeAdded](type=inet::Ipv6Route);
        @signal[routeChanged](type=inet::Ipv6Route);
        @signal[routeDeleted](type=inet::Ipv6Route);
        @signal[mrouteAdded](type=inet::Ipv6MulticastRoute);
        @signal[mrouteDeleted](type=inet::Ipv6MulticastRoute);
        @signal[mrouteChanged](type=inet::Ipv6MulticastRoute);
}

