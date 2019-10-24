
package inet.tutorials.queueing;

import inet.queueing.sink.ActivePacketSink;
import inet.queueing.filter.ContentBasedFilter;
import inet.queueing.source.PassivePacketSource;

network Filter2Network
{
    @display("bgb=600,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        filter: ContentBasedFilter {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider.out --> filter.in;
        filter.out --> collector.in;
}
