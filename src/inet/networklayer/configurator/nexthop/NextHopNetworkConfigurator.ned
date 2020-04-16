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
// Authors: Levente Meszaros (primary author), Andras Varga, Tamas Borbely
//

package inet.networklayer.configurator.nexthop;

import inet.networklayer.configurator.base.NetworkConfiguratorBase;

//
// Adds routes to a ~NextHopRoutingTable similarly how ~Ipv4NetworkConfigurator adds
// static routes to ~Ipv4RoutingTable.
//
simple NextHopNetworkConfigurator extends NetworkConfiguratorBase
{
    parameters:
        @class(NextHopNetworkConfigurator);
        @display("i=block/cogwheel");
        xml config = default(xml("<config/>")); // XML configuration parameters
        bool addStaticRoutes = default(true);  // add static routes to the routing tables of all nodes to route to all destination interfaces (only where applicable; turn off when config file contains manual routes)
        bool dumpTopology = default(false);  // print extracted network topology to the module output
        bool dumpRoutes = default(false);    // print configured and optimized routing tables for all nodes to the module output
}

