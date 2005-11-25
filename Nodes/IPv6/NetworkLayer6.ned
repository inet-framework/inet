//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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



import
    "IPv6",
    "ICMPv6",
    "IPv6NeighbourDiscovery",
    "IPv6ErrorHandling";


//
// Represents an IPv6 network layer (L3).
//
// The module has ports to connect to a higher layer (TCP,UDP) and
// several network interfaces.
//
module NetworkLayer6
    gates:
        in: ifIn[];
        out: ifOut[];
        in: TCPIn;
        out: TCPOut;
        in: UDPIn;
        out: UDPOut;
        in: RSVPIn;
        out: RSVPOut;
        in: OSPFIn;
        out: OSPFOut;
        in: pingIn;
        out: pingOut;
        out: errorOut;
    submodules:
        ipv6: IPv6;
            parameters:
                protocolMapping = "6:0,17:1,46:2,89:3";
            gatesizes:
                transportIn[4],
                transportOut[4],
                queueIn[sizeof(ifIn)],
                queueOut[sizeof(ifIn)];
            display: "i=block/network2;p=84,129";
        icmpv6: ICMPv6;
            display: "i=block/control;p=208,79";
        ipv6ErrorHandling: IPv6ErrorHandling;
            display: "p=280,79;i=block/process_s";
        neighbourDiscovery: IPv6NeighbourDiscovery;
            display: "p=208,171;i=block/network";
    connections nocheck:  // FIXME remove 'nocheck'!
        // IPv6 to transport Layer
        ipv6.transportOut[0] --> TCPOut display "m=n";
        ipv6.transportIn[0] <-- TCPIn display "m=n";

        ipv6.transportOut[1] --> UDPOut display "m=n";
        ipv6.transportIn[1] <-- UDPIn display "m=n";

        ipv6.transportOut[2] --> RSVPOut display "m=n";
        ipv6.transportIn[2] <-- RSVPIn display "m=n";

        ipv6.transportOut[3] --> OSPFOut display "m=n";
        ipv6.transportIn[3] <-- OSPFIn display "m=n";

        // IPv6 to ICMPv6
        ipv6.icmpOut --> icmpv6.fromIPv6;
        ipv6.icmpIn <-- icmpv6.toIPv6;

        // ICMPv6 to IPv6ErrorHandling
        icmpv6.errorOut --> ipv6ErrorHandling.in;

        // ICMPv6 to ping I/O
        icmpv6.pingOut --> pingOut;
        icmpv6.pingIn <-- pingIn;

        // IPv6 to Neighbour Discovery
        ipv6.ndOut --> neighbourDiscovery.fromIPv6;
        ipv6.ndIn <-- neighbourDiscovery.toIPv6;

        // IPv6 to L2
        for i=0..sizeof(ifOut)-1 do
            ifIn[i] --> ipv6.queueIn[i] display "m=s";
            ifOut[i] <-- ipv6.queueOut[i] display "m=s";
        endfor;
endmodule

