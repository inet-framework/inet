
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.filter.ContentBasedFilter;
import inet.queueing.source.ActivePacketSource;

network Filter3Network
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        filter: ContentBasedFilter {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> filter.in;
        filter.out --> consumer.in;
}

