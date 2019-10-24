
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.common.PacketDelayer;
import inet.queueing.source.ActivePacketSource;

network DelayerNetwork
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        delayer: PacketDelayer {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> delayer.in;
        delayer.out --> consumer.in;
}

