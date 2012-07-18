//
// Copyright (C) 2010 Philipp Berndt
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


package inet.networklayer.internetcloud;

//
// Configures IPv4 addresses and routing tables from a GNP file.
//
// This module does't connect to any other modules (it has no gates),
// and should have only one instance in the whole model. The module
// will only run once, at the beginning of the simulation.
//
// When it runs, it will:
//   -#  assign IPv4 addresses to nodes (e.g. hosts and routers).
//   -#  then it'll discover the topology of the network (using OMNeT++'s
//       cTopology class), and calculate shortest paths;
//   -#  finally, it will add routes which correspond to the shortest
//       paths to the routing tables (see RoutingTable::addRoutingEntry()).
//
// How does it know which modules are routers, hosts, etc. that need to
// be configured, and what is the network topology? The configurator
// picks all modules which have a @node property and their connections,
// and builds a graph from it. Then it runs Dijstra's
// shortest path algorithm on it, and configures all modules which are
// IP nodes.
//
// This module assign IPv4 address to a node only if the node has a 'group' parameter
// or a @group property: chooses an IP address for node from the selected group
// from XML file defined in gnpFile parameter.
// This module configures datarate and delay parameters of channels which
// connect group-node to InternetCloud module.
//
// It is assumed that the routing table (RoutingTable module) is the
// "routingTable" or "networkLayer.routingTable" submodule in all hosts
// and routers (all: not only the group-nodes).
//
// FIXME: csak a 'group' definicioval rendelkezo node-okra igaz a kovetkezo:
// To avoid interference with the above algorithm, it's recommended that
// no host or router should have its address set explicitly, and
// no routes are set up manually. Practically, routing files (.irt, .mrt)
// should be absent or empty.
//
// All the above takes place in initialization stage 2. (In stage 0,
// interfaces register themselves in the InterfaceTable modules, and
// in stage 1, routing files are read.)
//
simple InternetCloudNetworkConfigurator
{
    parameters:
        string gnpFile; // Filename of XML file containing embedded hosts and PingEr data
        @display("i=block/cogwheel_s");
        @labels(node);
}

