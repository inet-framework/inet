//
// Copyright (C) 2004 Andras Varga
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

package inet.networklayer.multi;

import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.generic.GenericNetworkLayer;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv6.IPv6NetworkLayer;


simple MultiNetworkLayerUpperMultiplexer
{
    parameters:
        @display("i=block/fork");

    gates:
        input transportUpperIn[];
        output transportUpperOut[];
        input transportLowerIn[];
        output transportLowerOut[];
        input pingUpperIn[];
        output pingUpperOut[];
        input pingLowerIn[];
        output pingLowerOut[];
}

simple MultiNetworkLayerLowerMultiplexer
{
    parameters:
        @display("i=block/fork");

    gates:
        input ifUpperIn[] @labels(INetworkDatagram);
        output ifUpperOut[] @labels(INetworkDatagram);
        input ifLowerIn[] @labels(INetworkDatagram);
        output ifLowerOut[] @labels(INetworkDatagram);
}

simple NetworkDatagramMultiplexer
{
    parameters:
        @display("i=block/fork");

    gates:
        input upperIn;
        output upperOut;
        input lowerIn[];
        output lowerOut[];
}

//
// This module supports multiple different network layers simultaneously.
//
module MultiNetworkLayer like INetworkLayer
{
    parameters:
        @display("i=block/fork");
        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        bool enableGeneric = default(true);
        ipv4.*.routingTableModule = default("^.^.^.routingTable.ipv4");
        ipv6.*.routingTableModule = default("^.^.^.routingTable.ipv6");
        generic.*.routingTableModule = default("^.^.^.routingTable.generic");

    gates:
        input ifIn[] @labels(INetworkDatagram);
        output ifOut[] @labels(INetworkDatagram);
        input transportIn[] @labels(IPv4ControlInfo/down);
        output transportOut[] @labels(IPv4ControlInfo/up);
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);

    submodules:
        ipv4: IPv4NetworkLayer if enableIPv4 {
            @display("p=100,300");
        }
        ipv6: IPv6NetworkLayer if enableIPv6 {
            @display("p=300,300");
        }
        generic: GenericNetworkLayer if enableGeneric {
            @display("p=500,300");
        }
        upperMultiplexer: MultiNetworkLayerUpperMultiplexer {
            @display("p=300,100");
        }
        lowerMultiplexer: MultiNetworkLayerLowerMultiplexer {
            @display("p=300,500");
        }

    connections allowunconnected:
        // transport
        for i=0..sizeof(transportIn)-1 {
            upperMultiplexer.transportLowerOut++ --> ipv4.transportIn++ if enableIPv4;
            upperMultiplexer.transportLowerOut++ --> ipv6.transportIn++ if enableIPv6;
            upperMultiplexer.transportLowerOut++ --> generic.transportIn++ if enableGeneric;
            transportIn[i] --> { @display("m=n"); } --> upperMultiplexer.transportUpperIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            ipv4.transportOut++ --> upperMultiplexer.transportLowerIn++ if enableIPv4;
            ipv6.transportOut++ --> upperMultiplexer.transportLowerIn++ if enableIPv6;
            generic.transportOut++ --> upperMultiplexer.transportLowerIn++ if enableGeneric;
            upperMultiplexer.transportUpperOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        // ping
        for i=0..sizeof(pingIn)-1 {
            upperMultiplexer.pingLowerOut++ --> ipv4.pingIn++ if enableIPv4;
            upperMultiplexer.pingLowerOut++ --> ipv6.pingIn++ if enableIPv6;
            upperMultiplexer.pingLowerOut++ --> generic.pingIn++ if enableGeneric;
            pingIn[i] --> { @display("m=n"); } --> upperMultiplexer.pingUpperIn++;
        }
        for i=0..sizeof(pingOut)-1 {
            ipv4.pingOut++ --> upperMultiplexer.pingLowerIn++ if enableIPv4;
            ipv6.pingOut++ --> upperMultiplexer.pingLowerIn++ if enableIPv6;
            generic.pingOut++ --> upperMultiplexer.pingLowerIn++ if enableGeneric;
            upperMultiplexer.pingUpperOut++ --> { @display("m=n"); } --> pingOut[i];
        }

        // interface
        for i=0..sizeof(ifIn)-1 {
            lowerMultiplexer.ifUpperOut++ --> ipv4.ifIn++ if enableIPv4;
            lowerMultiplexer.ifUpperOut++ --> ipv6.ifIn++ if enableIPv6;
            lowerMultiplexer.ifUpperOut++ --> generic.ifIn++ if enableGeneric;
            ifIn[i] --> { @display("m=s"); } --> lowerMultiplexer.ifLowerIn++;
        }
        for i=0..sizeof(ifOut)-1 {
            ipv4.ifOut++ --> lowerMultiplexer.ifUpperIn++ if enableIPv4;
            ipv6.ifOut++ --> lowerMultiplexer.ifUpperIn++ if enableIPv6;
            generic.ifOut++ --> lowerMultiplexer.ifUpperIn++ if enableGeneric;
            lowerMultiplexer.ifLowerOut++ --> { @display("m=s"); } --> ifOut[i];
        }
}
