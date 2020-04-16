//
// Copyright (C) 2012 Opensim Ltd.
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
//
// Authors: Levente Meszaros
//

package inet.networklayer.configurator.ipv4;

import inet.networklayer.configurator.contract.IIpv4NodeConfigurator;

//
// This module has one instance per network node, and it acts like a bridge
// between the node and the network's global configurator module,
// ~Ipv4NetworkConfigurator.
//
// ~Ipv4NetworkConfigurator only computes and remembers configuration data
// (assigned IP addresses, static routes, etc), but doesn't configure the nodes'
// routing tables and interfaces accordingly. The actual configuration is
// carried out by this module, at network initialization time and also when
// the node restarts (i.e. after a shutdown or a crash.)
//
// This module is needed because of failure/restart scenarios:
// ~Ipv4NetworkConfigurator would not be able to configure the node after
// a restart, because being a global module, it doesn't know about node
// lifecycle events.
//
simple Ipv4NodeConfigurator like IIpv4NodeConfigurator
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        // TODO: eventually rename to networkConfigurator
        string networkConfiguratorModule = default("configurator"); // the absolute path to the Ipv4NetworkConfigurator; use "" if there is no configurator
        bool configureRoutingTable = default(true);     // add routing entries to routing table (uses the configurator module)
        @display("i=block/cogwheel");
}

