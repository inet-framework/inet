//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.applications.dhcp;

import inet.applications.IUDPApp;

//
// The Dynamic Host Configuration Protocol (DHCP) provides configuration
// parameters to Internet hosts.  DHCP consists of two components: a
// protocol for delivering host-specific configuration parameters from a
// DHCP server to a host and a mechanism for allocation of network
// addresses to hosts.
//
// DHCP is built on a client (@DHCPClient)-server model, where designated
// DHCP server hosts allocate network addresses and deliver configuration
// parameters to dynamically configured hosts.
//
// For more info, see RFC 2131.
//
simple DHCPClient like IUDPApp
{
    parameters:
        string interface;  // which NIC should be configured (i.e. wlan0, eth1 etc.)
        string routingTablePath = default("^.routingTable");
        string interfaceTablePath = default("^.interfaceTable");
        string notificationBoardPath = default("^.notificationBoard");
        double startTime @unit(s) = default(0.5s); // starting time of DHCP initialization
        @display("i=block/app2");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut  @labels(UDPControlInfo/down);
}



