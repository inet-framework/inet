//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.tsn.streamfiltering.underthehood;

import inet.protocolelement.redundancy.StreamIdentifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network PeekingUnderTheHoodShowcase
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
        filter: <default("SimpleIeee8021qFilter")> like IPacketFilter {
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
