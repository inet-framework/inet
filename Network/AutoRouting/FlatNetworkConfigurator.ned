//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Configures \IP addresses and routing tables for a "flat" network,
// "flat" meaning that all hosts and routers will have the same
// network address and will only differ in the host part.
//
// This module does't connect to any other modules (it has no gates),
// and should have only one instance in the whole model. The module
// will only run once, at the beginning of the simulation.
// When it runs, it will:
//
//   -#  assign \IP addresses to hosts and routers. All hosts and
//       routers will be in the same network (same network address).
//       For simplicity, it will assign the same address to all interfaces
//       of a router;
//   -#  then it'll discover the topology of the network (using OMNeT++'s
//       cTopology class), and calculate shortest paths;
//   -#  finally, it will add routes which correspond to the shortest
//       paths to the routing tables (see RoutingTable::addRoutingEntry()).
//
// How does it know which modules are routers, hosts, et.c that need to
// be configured, and what is the network topology? The configurator
// picks all modules of types listed in the moduleTypes parameter and
// their connections, and builds a graph from it. Then it runs Dijstra's
// shortest path algorithm on it, and configures all modules which are
// \IP nodes, that is, not listed in the nonIPModuleTypes parameter.
//
// It is assumed that the routing table (RoutingTable module) is the
// "routingTable" or "networkLayer.routingTable" submodule in all hosts
// and routers.
//
// To avoid interference with the above algorithm, it's recommended that
// no host or router should have its address set explicitly, and
// no routes are set up manually. Practically, routing files (.irt, .mrt)
// should be missing or empty.
//
// All the above takes place in initialization stage 2. (In stage 0,
// interfaces register themselves in the InterfaceTable modules, and
// in stage 1, routing files are read.)
//
simple FlatNetworkConfigurator
    parameters:
        moduleTypes: string, // all module types to be considered part of the topology
        nonIPModuleTypes: string, // module types which don't need \IP configuration
                                  // (e.g. Ethernet hub, switch or bus)
        networkAddress: string, // network part of the address (see netmask parameter)
        netmask: string; // host part of addresses are autoconfigured
endsimple

