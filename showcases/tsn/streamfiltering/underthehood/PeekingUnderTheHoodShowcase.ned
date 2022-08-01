//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.tsn.streamfiltering.underthehood;

import inet.protocolelement.redundancy.StreamIdentifier;
import inet.queueing.classifier.ContentBasedClassifier;
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
        classifier: ContentBasedClassifier {
            @display("p=900,100");
        }
        sink[numSources]: PassivePacketSink {
            @display("p=1100,100,col,150");
        }
    connections:
        for i=0..numSources-1 {
            sources[i].out --> multiplexer.in++;
        }
        multiplexer.out --> identifier.in;
        identifier.out --> filter.in;
        filter.out --> classifier.in;
        for i=0..numSources-1 {
            classifier.out++ --> sink[i].in;
        }
}
