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

package inet.networklayer.autorouting.ipv4;

//
// This module provides the static configuration for the IPv4RoutingTable and
// the IPv4 network interfaces of a particular node. The configuration data
// is computed globally by the IPv4NetworkConfigurator module. The actual
// configuration happens during initialize and also when the node is starts up
// after a shut down or a crash.
//
simple IPv4NodeConfigurator
{
    parameters:
        @display("i=block/cogwheel_s");
        string networkConfiguratorModule = default("configurator"); // the absolute path to the IPv4NetworkConfigurator; use "" if there is no configurator
}
