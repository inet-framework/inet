//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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
// Edited with OMNeT++ Academic/Noncommercial Edition.
//




package inet.nodes.ipv6;

import inet.networklayer.icmpv6.ICMPv6;
import inet.networklayer.icmpv6.IPv6NeighbourDiscovery;
import inet.networklayer.ipv6.IPv6;
import inet.networklayer.ipv6.IPv6ErrorHandling;


//
// Represents an IPv6 network layer (L3).
//
// The module has ports to connect to a higher layer (TCP,UDP) and
// several network interfaces.
//
module NetworkLayer6
{
    parameters:
        @display("i=block/fork");
    gates:
        input ifIn[] @labels(IPv6Datagram);
        output ifOut[] @labels(IPv6Datagram);
        input tcpIn @labels(TCPSegment,IPv6ControlInfo/down);
        output tcpOut @labels(TCPSegment,IPv6ControlInfo/up);
        input udpIn @labels(UDPPacket,IPv6ControlInfo/down);
        output udpOut @labels(UDPPacket,IPv6ControlInfo/up);
        input rsvpIn @labels(IPv6ControlInfo/down);
        output rsvpOut @labels(IPv6ControlInfo/up);
        input ospfIn @labels(IPv6ControlInfo/down);
        output ospfOut @labels(IPv6ControlInfo/up);
        input pingIn;
        output pingOut;
        output errorOut;
    submodules:
        ipv6: IPv6 {
            parameters:
                protocolMapping = "6:0,17:1,46:2,89:3";
                @display("p=84,129");
            gates:
                transportIn[4];
                transportOut[4];
                queueIn[sizeof(ifIn)];
                queueOut[sizeof(ifIn)];
        }
        icmpv6: ICMPv6 {
            parameters:
                @display("p=208,79");
        }
        ipv6ErrorHandling: IPv6ErrorHandling {
            parameters:
                @display("p=280,79");
        }
        neighbourDiscovery: IPv6NeighbourDiscovery {
            parameters:
                @display("p=208,171");
        }
    connections allowunconnected:  // FIXME remove 'nocheck'!
        // IPv6 to transport Layer
        ipv6.transportOut[0] --> { @display("m=n"); } --> tcpOut;
        ipv6.transportIn[0] <-- { @display("m=n"); } <-- tcpIn;

        ipv6.transportOut[1] --> { @display("m=n"); } --> udpOut;
        ipv6.transportIn[1] <-- { @display("m=n"); } <-- udpIn;

        ipv6.transportOut[2] --> { @display("m=n"); } --> rsvpOut;
        ipv6.transportIn[2] <-- { @display("m=n"); } <-- rsvpIn;

        ipv6.transportOut[3] --> { @display("m=n"); } --> ospfOut;
        ipv6.transportIn[3] <-- { @display("m=n"); } <-- ospfIn;

        // IPv6 to ICMPv6
        ipv6.icmpOut --> icmpv6.ipv6In;
        ipv6.icmpIn <-- icmpv6.ipv6Out;

        // ICMPv6 to IPv6ErrorHandling
        icmpv6.errorOut --> ipv6ErrorHandling.in;

        // ICMPv6 to ping I/O
        icmpv6.pingOut --> pingOut;
        icmpv6.pingIn <-- pingIn;

        // IPv6 to Neighbour Discovery
        ipv6.ndOut --> neighbourDiscovery.ipv6In;
        ipv6.ndIn <-- neighbourDiscovery.ipv6Out;

        // IPv6 to L2
        for i=0..sizeof(ifOut)-1 {
            ifIn[i] --> { @display("m=s"); } --> ipv6.queueIn[i];
            ifOut[i] <-- { @display("m=s"); } <-- ipv6.queueOut[i];
        }
}

