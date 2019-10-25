
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.filter.PacketFilter;
import inet.queueing.source.ActivePacketSource;

network Filter1Network
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        filter: PacketFilter {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> filter.in;
        filter.out --> consumer.in;
}

