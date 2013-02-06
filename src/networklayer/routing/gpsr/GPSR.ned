//
// Copyright (C) 2004 Andras Varga
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

package inet.networklayer.routing.gpsr;

import inet.networklayer.multi.NetworkDatagramMultiplexer;


moduleinterface IGPSR
{
    parameters:
        @display("i=block/routing");
    gates:
        input ipIn;
        output ipOut;
}

//
// http://www.eecs.harvard.edu/~htk/publication/2000-mobi-karp-kung.pdf
//
simple GPSR like IGPSR
{
    parameters:
        // properties
        @display("i=block/routing");

        // context parameters
        string routingTableModule = default("routingTable");
        string networkProtocolModule = default("ip");

        // gpsr parameters
        int planarizationMode @enum(0,1) = default(0); // 0 for GG, 1 for RNG
        string interfaces = default("*");
        double beaconInterval @unit("s") = default(10s);
        double maxJitter @unit("s") = default(1s);
        double neighborValidityInterval @unit("s") = default(30s);

    gates:
        input ipIn;
        output ipOut;
}

module MultiGPSR like IGPSR
{
    parameters:
        @display("i=block/table");
        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        bool enableGeneric = default(true);
        ipv4GPSR.routingTableModule = default("ipv4RoutingTable");
        ipv4GPSR.networkProtocolModule = default("ip");
        ipv6GPSR.routingTableModule = default("ipv6RoutingTable");
        ipv6GPSR.networkProtocolModule = default("ipv6");
        genericGPSR.routingTableModule = default("genericRoutingTable");
        genericGPSR.networkProtocolModule = default("gnp");

    gates:
        input ipIn;
        output ipOut;

    submodules:
        // TODO: simplify names to ipv4, ipv6, generic once looking up modules support paths
        ipv4GPSR: GPSR if enableIPv4 {
            @display("p=100,100");
        }
        ipv6GPSR: GPSR if enableIPv6 {
            @display("p=200,100");
        }
        genericGPSR: GPSR if enableGeneric {
            @display("p=300,100");
        }
        multiplexer: NetworkDatagramMultiplexer {
            @display("p=200,200");
        }

    connections allowunconnected:
        multiplexer.upperOut --> ipOut;
        ipIn --> multiplexer.upperIn;
        multiplexer.lowerOut++ --> ipv4GPSR.ipIn if enableIPv4;
        ipv4GPSR.ipOut --> multiplexer.lowerIn++ if enableIPv4;
        multiplexer.lowerOut++ --> ipv6GPSR.ipIn if enableIPv6;
        ipv6GPSR.ipOut --> multiplexer.lowerIn++ if enableIPv6;
        multiplexer.lowerOut++ --> genericGPSR.ipIn if enableGeneric;
        genericGPSR.ipOut --> multiplexer.lowerIn++ if enableGeneric;
}
