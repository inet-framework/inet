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

package inet.examples.bgpv4.BGPOpen;

import inet.common.misc.MessageChecker;
import inet.node.inet.ApplicationLayerNodeBase;
import inet.routing.bgpv4.BGPRouting;


//
// Example IPv4 router with BGPv4 support.
//
module BGPRouterSimple extends ApplicationLayerNodeBase
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=abstract/router");
        forwarding = true;
    submodules:
        bgp: BGPRouting {
            parameters:
                ospfRoutingModule = default("");
                @display("p=364,50;i=block/network");
        }
        snifferIn: MessageChecker {
            parameters:
                @display("i=block/uparrow_s;p=333,124");
        }
        snifferOut: MessageChecker {
            parameters:
                @display("i=block/downarrow_s;p=397,124");
        }
    connections allowunconnected:
        bgp.socketOut --> { @display("m=s"); } --> snifferOut.in;
        bgp.socketIn <-- { @display("m=s"); } <-- snifferIn.out;

        snifferOut.out --> { @display("m=s"); } --> at.upperLayerIn++;
        snifferIn.in <-- { @display("m=s"); } <-- at.upperLayerOut++;
}

