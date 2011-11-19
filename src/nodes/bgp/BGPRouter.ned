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

package inet.nodes.bgp;

import inet.transport.ITCP;
import inet.nodes.ospfv2.OSPFRouter;
import inet.networklayer.bgpv4.BGPRouting;


//
// IP router with BGPv4 and OSPFv4 support.
//
module BGPRouter extends OSPFRouter
{
    parameters:
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
    submodules:
        bgp: BGPRouting {
            parameters:
                @display("p=239,33;i=block/network");
        }
        tcp: <tcpType> like ITCP {
            parameters:
                @display("p=161,72;i=block/transport");
        }
    connections allowunconnected:
        bgp.tcpOut --> tcp.appIn++;
        bgp.tcpIn <-- tcp.appOut++;

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;
}

