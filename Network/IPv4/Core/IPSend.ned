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


import     "NF_IP_LOCAL_OUT";



//
// Receives packet from Transport layer, ICMP or Tunneling (IP tunneled
// datagram) and encapsulates it into an IP datagram. Just encapsulates
// IPSendCore plus a queue hook.
//
module IPSend
    parameters:
        hookName : string,
        procdelay : numeric,
        timeToLive : numeric,
        multicastTimeToLive : numeric,
        numOfTransportProt : numeric const;
    gates:
        in: ICMPIn;
        in: tunnelIn;
        in: transportIn[];
        out: routingOut;
    submodules:
        core: IPSendCore;
            parameters:
                procdelay = procdelay,
                timeToLive = timeToLive,
                multicastTimeToLive = multicastTimeToLive,
                numOfTransportProt = numOfTransportProt;
            gatesizes:
                transportIn[numOfTransportProt];
            display: "p=125,212;b=40,24,rect";
        queueHook: hookName like NF_IP_LOCAL_OUT;
            display: "p=122,128;b=40,24,rect";
    connections nocheck:
        ICMPIn --> core.ICMPIn;
        tunnelIn --> core.tunnelIn;
        for i=0..numOfTransportProt-1 do
            transportIn[i] --> core.transportIn[i];
        endfor;
        core.routingOut --> routingOut;

        core.netfilterOut --> queueHook.in;
        queueHook.out --> core.netfilterIn;
endmodule


//
// Receives packet from Transport layer, ICMP or Tunneling (IP tunneled
// datagram) and encapsulates it into an IP datagram.
//  - set version
//  - set TOS (currently  constant 0!!!)
//  - set ds.codepoint
//  - set TTL
//  - choose and set fragmentation identifier
//  - set fragment offset = 0, more fragments = 0 (fragmentation occurs in IPFragmentation)
//  - set 'don't Fragment'-bit to received value (default 0)
//  - set Protocol to received value
//  - set destination address to received value
//  - send datagram to Routing
//  - if IPInterfacePacket is invalid (e.g. invalid source address), it is thrown away
//    without feedback
//
// Author: Jochen Reber
//
simple IPSendCore
    parameters:
        procdelay : numeric, // FIXME currently ignored!
        timeToLive : numeric,
        multicastTimeToLive : numeric,
        numOfTransportProt : numeric const;
    gates:
        in: ICMPIn;
        in: tunnelIn;
        in: transportIn[];
        in: netfilterIn;
        //#in: processorManagerIn;
        out: routingOut;
        out: netfilterOut;
endsimple


