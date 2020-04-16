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

package inet.routing.bgpv4;

import inet.routing.contract.IBgp;

//
// Implements BGP Version 4 routing protocol.
// 
// The bgpConfig parameter can specify an XML file describing the
// configuration of all BGP routers and their interfaces in the network.
//
//# TODO config file!
//
simple Bgp like IBgp
{
    parameters:
        string interfaceTableModule;
        string routingTableModule;
        string ospfRoutingModule;
        volatile double startupTime @unit(s) = default(0s); // delay before starting BGP
        xml bgpConfig;
        double ExternalPeerStartDelayOffset @unit(s) = default(0.5s);
        bool redistributeRip = default(false);        // redistribute routes learned by RIP into BGP
        string redistributeOspf = default("");        // redistribute routes learned by OSPF into BGP (O IA E1 E2)
        bool redistributeInternal = default(false);   // redistribute BGP routes into OSPF (as external routes)
        bool nextHopSelf = default(false);
        int localPreference = default(100);
        bool connectedCheck = default(true);
        int ebgpMultihop = default(1);
        @display("i=block/network2");
    gates:
        input socketIn;
        output socketOut;
}

