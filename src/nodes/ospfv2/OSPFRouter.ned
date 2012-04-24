//
// Copyright (C) 2005 Andras Babos
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


package inet.nodes.ospfv2;

import inet.base.NotificationBoard;
import inet.linklayer.IWiredNic;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.ospfv2.OSPFRouting;
import inet.nodes.inet.NetworkLayer;
import inet.nodes.inet.Router;


//
// An OSPFv2 router.
//
module OSPFRouter extends Router
{
    submodules:
        ospf: OSPFRouting {
            parameters:
                @display("p=465,287,row");
        }
    connections allowunconnected:
        ospf.ipOut --> networkLayer.ospfIn;
        ospf.ipIn <-- networkLayer.ospfOut;
}

