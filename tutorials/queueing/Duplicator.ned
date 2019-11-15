
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.common.PacketDuplicator;
import inet.queueing.source.ActivePacketSource;

network DuplicatorNetwork
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        duplicator: PacketDuplicator {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> duplicator.in;
        duplicator.out --> consumer.in;
}

