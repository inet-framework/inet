//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package inet.node.ipv6;

import inet.node.inet.NodeBase;


//
// BaseHost contains the common lower layers (linklayer and networklayer)
// of Router, StandardHost, WirelessHost etc.
//
// NOTE: - IPForward is turned on by default.
//       - use this model only as a base type. In simulations use Router, StandardHost,
//         WirelessHost and AndhocHost
//
module NodeBase6 extends NodeBase
{
    parameters:
        xml routes = default(xml("<routingTable/>"));
        networkLayerType = "IPv6NetworkLayer";
        routingTableType = "IPv6RoutingTable";
        routingTable.routes = routes;
}

