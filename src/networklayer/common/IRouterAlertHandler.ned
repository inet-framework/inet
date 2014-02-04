//
// Copyright (C) 2013 Markus Brueckner
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.networklayer.common;

// generic router alert handler interface
// This interface can be sub-classed by modules interested in intercepting
// IPv4 and IPv6 packets with the Router Alert option set. The interface
// offers two gates: one for receiving packets from the network layer (routerAlertIn)
// and one for re-injecting packets into the network layer (routerAlertReturn).
// Packets arriving at routerAlertIn are handed over to the module completely.
// If the packets are to be sent onwards along the normal routing path, they
// have to be re-injected via the routerAlertReturn gate.
moduleinterface IRouterAlertHandler
{
    parameters:
        @display("i=block/control_s");    
	gates:
	    input  routerAlertIn @labels(IPv4Datagram,IPv6Datagram);
	    output routerAlertReturn @labels(IPv4Datagram,IPv6Datagram);
}