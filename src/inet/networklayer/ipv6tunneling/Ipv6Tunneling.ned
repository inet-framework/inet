//
// Copyright (C) 2007
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.networklayer.ipv6tunneling;

import inet.networklayer.contract.IIpv6Tunneling;

//
// IPv6 Tunnel Manager
//
// @see ~Ipv6, ~Ipv6NeighbourDiscovery, ~Icmpv6
//
simple Ipv6Tunneling like IIpv6Tunneling
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string xmipv6Module;
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input linkLayerIn;
        output linkLayerOut;
}

