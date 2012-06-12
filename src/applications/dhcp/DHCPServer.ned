//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
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

// Simple DHCP Server
//
// Only assign a simple network with a exclusion list
//
simple DHCPServer like IUDPApp
{
    parameters:
        string net;  		// IP network to assign
        string mask;  		// netmask to assign
        string ip_begin;	// ip to start to lease
        int client_num; 	// number of ip to lease
        string gateway;		// gateway to assign
        string dns;			// dns to assign
        int lease_time;		// lease time in seconds
        string iface;		// interface to listen
        @display("i=device/card");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut  @labels(UDPControlInfo/down);
}



