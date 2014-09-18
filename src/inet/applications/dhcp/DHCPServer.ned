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
// Implements the DHCP server protocol. DHCP (Dynamic Host Configuration Protocol),
// described in RFC 2131, provides configuration parameters to Internet hosts.
// Requires UDP.
//
// @see DHCPClient, DHCPMessage
//
simple DHCPServer like IUDPApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string interface = default(""); // interface to listen on; can be left empty if there is only one non-loopback interface
        string subnetMask; // subnet mask to assign
        string ipAddressStart; // IP pool 1st address
        int maxNumClients; // maximum number of clients (IPs) allowed to be leased
        string gateway; // gateway to assign
        string dns; // DNS server to assign
        int leaseTime @unit(s); // lease time in seconds
        double startTime @unit(s) = default(0s); // application start time
        @display("i=block/app2");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}



