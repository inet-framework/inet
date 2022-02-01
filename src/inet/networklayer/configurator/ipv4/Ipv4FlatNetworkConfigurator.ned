//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.ipv4;

import inet.networklayer.configurator.contract.IL3NetworkConfigurator;

//
// Configures IPv4 addresses and routing tables for a "flat" network,
// "flat" meaning that all hosts and routers will have the same
// network address and will only differ in the host part.
//
// This module does't connect to any other modules (it has no gates),
// and should have only one instance in the whole model. The module
// will only run once, at the beginning of the simulation.
// When it runs, it will:
//
//   -#  assign IPv4 addresses to hosts and routers. All hosts and
//       routers will be in the same network (same network address).
//       For simplicity, it will assign the same address to all interfaces
//       of a router;
//   -#  then it'll discover the topology of the network (using OMNeT++'s
//       cTopology class), and calculate shortest paths;
//   -#  finally, it will add routes which correspond to the shortest
//       paths to the routing tables (see Ipv4RoutingTable::addRoutingEntry()).
//
// How does it know which modules are routers, hosts, et.c that need to
// be configured, and what is the network topology? The configurator
// picks all modules which have a @networkNode property and their connections,
// and builds a graph from it. Then it runs Dijstra's
// shortest path algorithm on it, and configures all modules which are
// IPv4 nodes.
//
// It is assumed that the routing table (~Ipv4RoutingTable module) is the
// "routingTable" or "networkLayer.routingTable" submodule in all hosts
// and routers.
//
// To avoid interference with the above algorithm, it's recommended that
// no host or router should have its address set explicitly, and
// no routes are set up manually. Practically, routing files (.irt, .mrt)
// should be absent or empty.
//
// All the above takes place in initialization stage 2. (In stage 0,
// interfaces register themselves in the ~InterfaceTable modules, and
// in stage 1, routing files are read.)
//
simple Ipv4FlatNetworkConfigurator like IL3NetworkConfigurator
{
    parameters:
        string networkAddress = default("192.168.0.0"); // network part of the address (see netmask parameter)
        string netmask = default("255.255.0.0"); // host part of addresses are autoconfigured
        @display("i=block/cogwheel");
        @labels(node);
}

