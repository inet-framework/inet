// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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



import "NF_IP_FORWARD";


//
// Routing module.
//
// Receive correct IP datagram:
// - if source routing option is on, use next source addr. as dest. addr.
// - map IP address on output port, use static routing table
// - if destination address is not in routing table, 
//   throw datagram away and notify ICMP
// - process record route and timestamp options, if applicable
// - send to local Deliver if dest. addr. = 127.0.0.1 
//   or dest. addr. = NetworkCardAddr.[]
// - send datagram with Multicast addr. to Multicast module
// - otherwise, send to Fragmentation module
//
// author: Jochen Reber
// date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
//
module Routing 
    parameters: 
        hookName : string,
        procdelay : numeric const,
        IPForward : bool; 
    gates: 
        in: preRoutingIn; 
        in: sendIn; 
        out: localOut; 
        out: fragmentationOut; 
        out: multicastOut; 
        out: errorOut; 
    submodules: 
        core: RoutingCore; 
            parameters:
                procdelay = procdelay,
                IPForward = IPForward;
            display: "p=131,186;b=40,24,rect";
        queueHook: hookName like NF_IP_FORWARD; 
            display: "p=133,102;b=40,24,rect";
    connections: 
        preRoutingIn --> core.preRoutingIn; 
        sendIn --> core.sendIn; 
        core.localOut --> localOut; 
        core.fragmentationOut --> fragmentationOut; 
        core.multicastOut --> multicastOut; 
        core.errorOut --> errorOut; 
                                    
        core.netfilterOut --> queueHook.in; 
        queueHook.out --> core.netfilterIn; 
    display: "p=10,10;b=260,235,rect";
endmodule 

simple RoutingCore 
    parameters: 
        procdelay : numeric const,
        IPForward : bool; 
    gates: 
        in: preRoutingIn; 
        in: sendIn; 
        in: netfilterIn; 
        out: localOut; 
        out: fragmentationOut; 
        out: multicastOut; 
        out: errorOut; 
        out: netfilterOut; 
endsimple 

