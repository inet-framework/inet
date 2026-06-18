//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.ipv6;

import inet.networklayer.configurator.base.L3NetworkConfiguratorBase;

//
// Assigns IPv6 addresses and sets up static routing for an IPv6 network.
// It assigns per-interface global IPv6 addresses based on prefix templates,
// and can configure static routes using Dijkstra's shortest path algorithm.
//
// Whether host interfaces are addressed statically or left to autoconfigure is
// controlled by the `assignAddressesToHosts` parameter:
//
//  - `assignAddressesToHosts = true` (default): the configurator directly
//    assigns global addresses to every interface (hosts and routers) and adds
//    static routes to the routing tables. Neighbor Discovery is not needed for
//    address assignment.
//
//  - `assignAddressesToHosts = false`: the configurator addresses only router
//    interfaces and sets up their advertised prefixes (AdvPrefix); host
//    interfaces then obtain their global addresses through dynamic
//    configuration -- Stateless Address Autoconfiguration (SLAAC) today, or
//    potentially DHCPv6 via the Managed/Other-config RA flags. The dynamic
//    method is selected by the usual mechanisms, not by this parameter. Static
//    routes are still added, including on-link routes for the router links that
//    such hosts attach to. This mode is required for mobile nodes (MIPv6),
//    whose home/care-of addresses must be autoconfigured.
//
// Independently, `assignAddresses = false` disables address assignment
// entirely, and `addStaticRoutes = false` disables static route generation.
//
// Network nodes are supposed to contain an instance of ~Ipv6NodeConfigurator
// (normally part of the network layer compound module) that actually
// configures the node's interface table and routing table based on the
// information stored in the global network configurator module.
//
// The configurator supports hierarchical addressing via XML prefix templates.
// Each link gets a unique /64 prefix (required for SLAAC compatibility),
// and prefixes are allocated from hierarchical blocks specified in the XML.
// For example, all hosts in "area1" can be assigned prefixes from
// 2001:db8:1::/48, resulting in prefixes like 2001:db8:1:1::/64,
// 2001:db8:1:2::/64, etc.
//
// The XML configuration format mirrors that of ~Ipv4NetworkConfigurator
// with adapted attributes. Under the root `<config>` element:
//
//  - `<interface>`
//    Assigns prefixes to interfaces.
//
//    - `@hosts` - host name pattern selector (default: "**")
//    - `@names` - interface name pattern selector (default: "*")
//    - `@towards` - neighbor host name pattern selector (default: "*")
//    - `@among` - shorthand for hosts+towards
//    - `@prefix` - prefix template, e.g. "2001:db8:1:x::/64". Use 'x' for
//      unspecified 16-bit groups that will be filled by the configurator.
//    - `@address` - explicit full IPv6 address for the interface, e.g.
//      "2001:db8:1:1::1". Overrides the EUI-64-derived interface id; must lie
//      within the interface's link prefix. With no `@prefix` on the link, the
//      address also seeds the link's /64 prefix. Use this when an interface
//      needs a specific address (e.g. an RFC 3956 embedded-RP "prefix::RIID").
//    - `@mtu` - optional MTU override
//    - `@metric` - optional metric override
//    - `@add-static-route` - optional bool (default true)
//    - `@add-default-route` - optional bool (default true)
//    - `@advValidLifetime` - AdvPrefix valid lifetime in seconds (default: 2592000 = 30 days)
//    - `@advPreferredLifetime` - AdvPrefix preferred lifetime in seconds (default: 604800 = 7 days)
//    - `@advOnLinkFlag` - AdvPrefix on-link flag (default: true)
//    - `@advAutonomousFlag` - AdvPrefix autonomous flag for SLAAC (default: true)
//    - `@advManagedFlag` - "Managed address configuration" (M) flag in RAs (default: false)
//    - `@advOtherConfigFlag` - "Other configuration" (O) flag in RAs (default: false)
//    - `@maxRtrAdvInterval` - max time between unsolicited RAs in seconds (default: 600)
//    - `@minRtrAdvInterval` - min time between unsolicited RAs in seconds (default: 200)
//    - `@advDefaultLifetime` - Router Lifetime in RAs in seconds (default: 3*maxRtrAdvInterval)
//    - `@dupAddrDetectTransmits` - number of NS sent for DAD (default: 1; 0 = disable DAD)
//
//  - `<route>`
//    Adds manual static routes.
//
//    - `@hosts` - host name pattern selector
//    - `@destination` - destination prefix (L3AddressResolver syntax); may include
//      the prefix length in "address/prefixlen" notation (e.g. "2001:db8::/64")
//    - `@prefixLength` - prefix length (default: 64); must not be combined with the
//      "/prefixlen" notation in `@destination`
//    - `@gateway` - next-hop address (L3AddressResolver syntax)
//    - `@interface` - output interface name
//    - `@metric` - route metric (default: 0)
//
//  - `<autoroute>`
//    Parameters for automatic static route generation.
//
//    - `@sourceHosts` - source host pattern (default: "**")
//    - `@destinationInterfaces` - destination interface pattern (default: "**")
//    - `@metric` - "hopCount", "delay", "dataRate", or "errorRate" (default: "hopCount")
//
simple Ipv6NetworkConfigurator extends L3NetworkConfiguratorBase
{
    parameters:
        xml config = default(xml("<config><interface hosts='**' prefix='2001:db8:x:x::/64'/></config>")); // XML configuration parameters for IPv6 prefix assignment and manual routes
        bool assignAddresses = default(true); // Assign IPv6 global addresses to interfaces
        bool assignAddressesToHosts = default(true); // If true, statically assign global addresses to host (non-router) interfaces too. If false, configure only router interfaces and their advertised prefixes, leaving hosts to obtain addresses through DYNAMIC configuration; which dynamic method is used is selected by the usual mechanisms (router AdvAutonomousFlag for SLAAC; the Managed/Other-config RA flags plus a DHCPv6 server for DHCPv6), not by this parameter. Set to false for mobile nodes (MIPv6), whose home/care-of addresses must be autoconfigured. Routers always get static addresses regardless of this setting.
        bool addStaticRoutes = default(true); // Add static routes to all routing tables
        bool addDefaultRoutes = default(true); // Add default routes if all routes from a source go through the same gateway
        bool addDirectRoutes = default(true); // Add direct routes (on-link prefix routes) for local network members
        bool dumpTopology = default(false); // Print extracted network topology
        bool dumpLinks = default(false); // Print recognized network links
        bool dumpAddresses = default(false); // Print assigned IPv6 addresses for all interfaces
        bool dumpRoutes = default(false); // Print configured routing tables
        @class(Ipv6NetworkConfigurator);
        @display("i=block/cogwheel");
        @signal[networkConfigurationChanged];
}


