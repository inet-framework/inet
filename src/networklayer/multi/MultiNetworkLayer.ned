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
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv6.IPv6NetworkLayer;
import inet.networklayer.generic.GenericNetworkLayer;

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
// TODO
//
module MultiNetworkLayer like INetworkLayer
{
    parameters:
        @display("i=block/fork");
        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        bool enableGeneric = default(true);
        ipv4NetworkLayer.*.routingTableModuleName = default("ipv4RoutingTable");
        ipv6NetworkLayer.*.routingTableModuleName = default("ipv6RoutingTable");
        genericNetworkLayer.*.routingTableModuleName = default("genericRoutingTable");

    gates:
        input ifIn[] @labels(INetworkDatagram);
        output ifOut[] @labels(INetworkDatagram);
        input transportIn[] @labels(IPv4ControlInfo/down);
        output transportOut[] @labels(IPv4ControlInfo/up);
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);

    submodules:
        // TODO: simplify names to ipv4, ipv6, generic once looking up modules support paths
        ipv4NetworkLayer: IPv4NetworkLayer if enableIPv4 {
            @display("p=100,300");
        }
        ipv6NetworkLayer: IPv6NetworkLayer if enableIPv6 {
            @display("p=300,300");
        }
        genericNetworkLayer: GenericNetworkLayer if enableGeneric {
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
            upperMultiplexer.transportLowerOut++ --> ipv4NetworkLayer.transportIn++ if enableIPv4;
            upperMultiplexer.transportLowerOut++ --> ipv6NetworkLayer.transportIn++ if enableIPv6;
            upperMultiplexer.transportLowerOut++ --> genericNetworkLayer.transportIn++ if enableGeneric;
            transportIn[i] --> { @display("m=n"); } --> upperMultiplexer.transportUpperIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            ipv4NetworkLayer.transportOut++ --> upperMultiplexer.transportLowerIn++ if enableIPv4;
            ipv6NetworkLayer.transportOut++ --> upperMultiplexer.transportLowerIn++ if enableIPv6;
            genericNetworkLayer.transportOut++ --> upperMultiplexer.transportLowerIn++ if enableGeneric;
            upperMultiplexer.transportUpperOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        // ping
        for i=0..sizeof(pingIn)-1 {
            upperMultiplexer.pingLowerOut++ --> ipv4NetworkLayer.pingIn++ if enableIPv4;
            upperMultiplexer.pingLowerOut++ --> ipv6NetworkLayer.pingIn++ if enableIPv6;
            upperMultiplexer.pingLowerOut++ --> genericNetworkLayer.pingIn++ if enableGeneric;
            pingIn[i] --> { @display("m=n"); } --> upperMultiplexer.pingUpperIn++;
        }
        for i=0..sizeof(pingOut)-1 {
            ipv4NetworkLayer.pingOut++ --> upperMultiplexer.pingLowerIn++ if enableIPv4;
            ipv6NetworkLayer.pingOut++ --> upperMultiplexer.pingLowerIn++ if enableIPv6;
            genericNetworkLayer.pingOut++ --> upperMultiplexer.pingLowerIn++ if enableGeneric;
            upperMultiplexer.pingUpperOut++ --> { @display("m=n"); } --> pingOut[i];
        }

        // interface
        for i=0..sizeof(ifIn)-1 {
            lowerMultiplexer.ifUpperOut++ --> ipv4NetworkLayer.ifIn++ if enableIPv4;
            lowerMultiplexer.ifUpperOut++ --> ipv6NetworkLayer.ifIn++ if enableIPv6;
            lowerMultiplexer.ifUpperOut++ --> genericNetworkLayer.ifIn++ if enableGeneric;
            ifIn[i] --> { @display("m=s"); } --> lowerMultiplexer.ifLowerIn++;
        }
        for i=0..sizeof(ifOut)-1 {
            ipv4NetworkLayer.ifOut++ --> lowerMultiplexer.ifUpperIn++ if enableIPv4;
            ipv6NetworkLayer.ifOut++ --> lowerMultiplexer.ifUpperIn++ if enableIPv6;
            genericNetworkLayer.ifOut++ --> lowerMultiplexer.ifUpperIn++ if enableGeneric;
            lowerMultiplexer.ifLowerOut++ --> { @display("m=s"); } --> ifOut[i];
        }
}
