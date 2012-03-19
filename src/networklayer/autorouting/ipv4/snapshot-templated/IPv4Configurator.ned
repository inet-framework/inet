//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.networklayer.autorouting.ipv4;

//
// This module configures IPv4 addresses and routing tables for a network.
//
// The configurator doesn't connect to any other modules (it has no gates),
// and should have only one instance in the whole model.  The configuration
// takes place in initialization stage 2 after the interfaces are registered
// in the ~InterfaceTable modules.
// 
// The network consists of nodes that are either hosts or routers. The
// configurator recognizes the modules as nodes that have a @node property. 
// All nodes must have their interface table (~InterfaceTable module) as their
// "interfaceTable" submodule. All routers must have their routing table
// (~RoutingTable module) as their "routingTable" or "networkLayer.routingTable"
// submodule.
//
// By default the resulting configuration will have the following properties.
// All interfaces in all nodes will have a unique IPv4 address assigned. If
// there are multiple interfaces in a single host then each one of them will
// also have a unique IPv4 address. Routing tables will be configured throughout
// the whole network so that there is a route following the shortest path from
// any node to any interface. In other words, all nodes will be able to ping
// all interfaces in the network. 
// 
// The configurator operates in the following way.
//  -# It first builds a graph representing the network topology. The graph
//     will have a vertex for all modules that have a @node property including
//     hosts, hubs, buses, bridges, switches, access points and routers. The
//     vertex weight will be zero for routers that have IP forwarding enabled.
//     In all other nodes with a routing table it will be infinite to prevent
//     transit traffic. The graph will have an edge for all connections that
//     connect two network interfaces. The edge weight will be inverse
//     proportional to the bitrate of the connection. Thus, the configurator
//     will prefer connections with higher bandwidth.
//  -# Then it assigns IP addresses to all interfaces of all nodes. The
//     assignment process takes into consideration the addresses and netmasks
//     already present on the interfaces (possibly set in earlier initialize
//     stages), and the configuration provided in the XML format (described
//     below). In the most general case the configurator is allowed to choose
//     any address and netmask for all interfaces. In the most limited case the
//     configurator is forced to use the requested addresses and netmasks for
//     all interfaces. There are many possible configuration options between
//     these two extremums. The configurator assigns addresses in a way that
//     maximizes the number of nodes per subnet. Once it figured out the nodes
//     that belong to a signle subnet it will optimzie for allocating the
//     longest possible netmask. The configurator might fail to assign netmasks
//     and addresses according to the given configuraiton parameters. If this
//     happens then the assignment process stops and an error is signalled. 
//  -# Then it adds static routes to all routing tables in the network. The
//     configurator uses Dijstra's weighted shortest path algorithm to find
//     the desired routes between all possible node pairs. The resulting
//     routing tables will have one entry for all destination interfaces in the
//     network. The configurator can be safely instructed to add default routes
//     where applicable, significantly reducing the size of the host routing
//     tables. It can also add subnet routes instead of interface routes further
//     reducing the size of routing tables. Turing on this option requires
//     careful design to avoid having IP addresses from the same subnet on
//     different links. 
//  -# Then it optimizes the routing tables for size. This optimization allows
//     configuring larger networks with smaller memory footprint and makes the
//     routing table lookup faster. The resulting routing table might be
//     different in that it will route packets that the original routing table
//     did not. Nevertheless the following invariant holds: any packet routed
//     by the original routing table (has matching route) will still be routed
//     the same way by the optimized routing table.  
//  -# Finally it dumps the requested results of the configuration. It can
//     dump network topology, assigned IP addresses, routing tables and its
//     own configuration format.
//
// The XML configuration must contain exactly one <config> element. Under the
// root element there can be multiple of the following elements. 
//  - <interface>
//    The interface element provides configuration parameters for one or more
//    interfaces in the network. The selector attributes limit the scope where
//    the interface element has effects. The parameter attributes limit the
//    range of assignable addresses and netmasks.
//    - @hosts
//      Selector attribute that specifies a list of host name patterns. Only
//      interfaces in the specified hosts are affected.
//      e.g. "host* router[0..3]"
//    - @names
//      Selector attribute that specifies a list of interface name patterns.
//      Only interfaces with the specified names are affected.
//      e.g. "eth* ppp0"
//    - @towards
//      Selector attribute that specifies a list of host name patterns. Only
//      interfaces connected towards the specified hosts are affected.
//      e.g. "ap switch"
//    - @configure
//      Parameter attribute that specifies whether the interface will be
//      configured or not. Unconfigured interfaces still have allocated
//      addresses in their subnets allowing them to become configured later
//      very easily. The default value is "true".
//    - @address
//      Parameter attribute that limits the range of assignable addresses.
//      Wildcards are allowed with using 'x' as part of the address. Unspecified
//      parts will be filled automatically be the configurator.
//      e.g. "192.168.1.1" or "10.0.x.x"
//    - @netmask
//      Parameter attribute that limits the range of assignable netmasks.
//      Wildcards are allowed with using 'x' as part of the netmask. Unspecified
//      parts will be filled automatically be the configurator.
//      e.g. "255.255.255.0" or "255.255.x.x" or "255.255.x.0"
//    - @multicastgroups
//      TODO
//      e.g. "224.0.0.1 224.0.1.33"
//    - @mtu                number
//      Parameter attribute to set the MTU parameter in the interface.
//  - <route>
//    The route element provides routing table entries for one or more nodes
//    in the network. The selector attributes limit the scope where the route
//    element has effects. The effect of route elements is unspecified if the
//    configurator is instructed to add static routes automatically.
//    - @host
//      Selector attribute that specifies a list of host name patterns. Only
//      routing tables in the specified hosts are affected.
//      e.g. "host* router[0..3]"
//    - @destination
//      Parameter attribute that specifies the destination address in the route 
//      (~IPvXAddressResolver syntax).
//    - @netmask
//      Parameter attribute that specifies the netmask in the route. The default
//      value is "255.255.255.255".
//      e.g. "255.255.255.0" or "/24" 
//    - @gateway
//      Parameter attribute that specifies the gateway address in the route 
//      (~IPvXAddressResolver syntax).
//    - @interface
//      Parameter attribute that specifies the output interface name in the
//      route.
//    - @metric
//      Parameter attribute that specifies the metric in the route.
//
simple IPv4Configurator extends IPConfigurator
{
    parameters:
        @class(IPv4Configurator);
        xml config = default(xml("<config><interface hosts='*' address='10.x.x.x' netmask='255.x.x.x'/></config>")); // XML configuration parameters for IP address assignment and adding manual routes
        bool addStaticRoutes = default(true);  // adds static routes to the routing tables of all nodes to route to all destination interfaces (only where applicable; turn off when config file contains manual routes)
        bool addDefaultRoutes = default(true); // adds default routes if all routes from a source node go through the same gateway (used only if addStaticRoutes is true)
        bool addSubnetRoutes = default(true);  // adds subnet routes instead of destination interface routes (only where applicable; used only if addStaticRoutes is true)
        bool optimizeRoutes = default(true); // optimizes routing tables by merging routes, the resulting routing table might route more packets than the original (used only if addStaticRoutes is true)
        bool dumpTopology = default(false);  // prints extracted network topology to the module output
        bool dumpRoutes = default(false);    // prints configured and optimized routing tables for all nodes to the module output
        string dumpConfig = default("");     // writes configuration into the given config file that can be fed back to speed up subsequent runs (network configurations)
}
