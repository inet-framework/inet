//
// Copyright (C) 2010 Helene Lageber
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

package inet.examples.bgpv4.BGPUpdate;

import inet.common.misc.MessageChecker;
import inet.examples.bgpv4.BGPOpen.BGPRouterSimple;
import inet.linklayer.contract.IWiredNic;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.routing.bgpv4.BGPRouting;
import inet.routing.ospfv2.OSPFRouting;
import inet.transportlayer.contract.ITCP;


//
// Example IP router with BGPv4 and OSPFv4 support.
//
module BGPRouterEx extends BGPRouterSimple
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        bgp.ospfRoutingModule = default("^.ospf");
    submodules:
        ospf: OSPFRouting {
            parameters:
                @display("p=800,300,row;i=block/network2");
        }
    connections allowunconnected:
        ospf.ipOut --> tn.upperLayerIn++;
        ospf.ipIn <-- tn.upperLayerOut++;
}

