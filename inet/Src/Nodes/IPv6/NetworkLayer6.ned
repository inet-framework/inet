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
//
// Edited with OMNeT++ Academic/Noncommercial Edition.
//




package org.omnetpp.inet.Nodes.IPv6;

import org.omnetpp.inet.Network.ICMPv6.ICMPv6;
import org.omnetpp.inet.Network.ICMPv6.IPv6NeighbourDiscovery;
import org.omnetpp.inet.Network.IPv6.IPv6;
import org.omnetpp.inet.Network.IPv6.IPv6ErrorHandling;


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
        input ifIn[];
        output ifOut[];
        input TCPIn;
        output TCPOut;
        input UDPIn;
        output UDPOut;
        input RSVPIn;
        output RSVPOut;
        input OSPFIn;
        output OSPFOut;
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
        ipv6.transportOut[0] --> { @display("m=n"); } --> TCPOut;
        ipv6.transportIn[0] <-- { @display("m=n"); } <-- TCPIn;

        ipv6.transportOut[1] --> { @display("m=n"); } --> UDPOut;
        ipv6.transportIn[1] <-- { @display("m=n"); } <-- UDPIn;

        ipv6.transportOut[2] --> { @display("m=n"); } --> RSVPOut;
        ipv6.transportIn[2] <-- { @display("m=n"); } <-- RSVPIn;

        ipv6.transportOut[3] --> { @display("m=n"); } --> OSPFOut;
        ipv6.transportIn[3] <-- { @display("m=n"); } <-- OSPFIn;

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
        for i=0..sizeof(ifOut)-1 {
            ifIn[i] --> { @display("m=s"); } --> ipv6.queueIn[i];
            ifOut[i] <-- { @display("m=s"); } <-- ipv6.queueOut[i];
        }
}

