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
// This module supports two modes of operation:
//
//  - **Mode A (static configuration)**: The configurator directly assigns
//    global addresses to all interfaces and adds static routes to routing
//    tables. Neighbor Discovery Protocol (NDP) is not needed for address
//    assignment. This is the default mode.
//
//  - **Mode B (prefix advertisement)**: The configurator only sets up
//    advertised prefix (AdvPrefix) entries on router interfaces. Host
//    interfaces then obtain their global addresses via Stateless Address
//    Autoconfiguration (SLAAC) triggered by Router Advertisements. Static
//    routes are still added by the configurator.
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
//    - `@mtu` - optional MTU override
//    - `@metric` - optional metric override
//    - `@add-static-route` - optional bool (default true)
//    - `@add-default-route` - optional bool (default true)
//
//  - `<route>`
//    Adds manual static routes.
//
//    - `@hosts` - host name pattern selector
//    - `@destination` - destination prefix (L3AddressResolver syntax)
//    - `@prefixLength` - prefix length (default: 64)
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
        bool assignAddresses = default(true); // Assign IPv6 global addresses to all interfaces
        bool configureAdvPrefixes = default(false); // Mode B: only set AdvPrefix on router interfaces, let hosts use SLAAC (if false, Mode A: directly assign addresses)
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


