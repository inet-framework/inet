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


import     "NF_IP_LOCAL_IN";


//
// Handles local delivery of IP datagrams, the main task being defragmentation.
//
// Receive IP datagram for local delivery:
// - strip off IP header
// - buffer fragments for ip_fragmenttime
// - wait until all fragments of one fragment number are received
// - discard without notification if not all fragments arrive in ip_fragmenttime
//
// Once all fragments have arrived:
// - send Transport packet up to the transport layer
// - send ICMP packet to ICMP module
// - send IGMP group management packet to Multicast module
// - send tunneled IP datagram to PreRouting
//
// author: Jochen Reber
// date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
//
module LocalDeliver
    parameters:
        procdelay : numeric,
        numOfTransportProt : numeric const,
        hookName : string,
        fragmentTimeout : string;
    gates:
        in: routingIn;
        in: multicastIn;
        out: multicastOut;
        out: ICMPOut;
        out: preRoutingOut;
        out: transportOut[];
    submodules:
        core: LocalDeliverCore;
            parameters:
                procdelay = procdelay,
                numOfTransportProt = numOfTransportProt,
                fragmentTimeout = fragmentTimeout;
            gatesizes:
                transportOut[numOfTransportProt];
            display: "p=101,180;b=40,24,rect";
        queueHook: hookName like NF_IP_LOCAL_IN;
            display: "p=100,110;b=40,24,rect";
    connections nocheck:
        routingIn --> core.routingIn;
        multicastIn --> core.multicastIn;
        core.multicastOut --> multicastOut;
        core.ICMPOut --> ICMPOut;
        core.preRoutingOut --> preRoutingOut;
        for i=0..numOfTransportProt-1 do
            core.transportOut[i] --> transportOut[i];
        endfor;

        core.netfilterOut --> queueHook.in;
        queueHook.out --> core.netfilterIn;
    display: "p=10,18;b=202,224,rect";
endmodule

simple LocalDeliverCore
    parameters:
        procdelay : numeric,
        numOfTransportProt : numeric const,
        fragmentTimeout : string;
    gates:
        in: routingIn;
        in: multicastIn;
        in: netfilterIn;
        //in: processorManagerIn;
        out: multicastOut;
        out: ICMPOut;
        out: preRoutingOut;
        out: transportOut[];
        out: netfilterOut;
endsimple
