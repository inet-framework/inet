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

package inet.examples.ethernet.filtering;

import inet.linklayer.ieee8021q.Ieee8021qciFilter;
import inet.protocolelement.redundancy.StreamIdentifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network EthernetFilteringExample
{
    parameters:
        int numSources;
    submodules:
        sources[numSources]: ActivePacketSource {
            @display("p=100,100,column,150");
        }
        multiplexer: PacketMultiplexer {
            @display("p=300,100");
        }
        identifier: StreamIdentifier {
            @display("p=500,100");
        }
        filter: Ieee8021qciFilter {
            @display("p=700,100");
        }
        sink: PassivePacketSink {
            @display("p=900,100");
        }
    connections:
        for i=0..numSources-1 {
            sources[i].out --> multiplexer.in++;
        }
        multiplexer.out --> identifier.in;
        identifier.out --> filter.in;
        filter.out --> sink.in;
}
