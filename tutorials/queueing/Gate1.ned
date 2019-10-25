
package inet.tutorials.queueing;

import inet.queueing.common.PacketGate;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network Gate1Network
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        gate: PacketGate {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections:
        producer.out --> gate.in;
        gate.out --> consumer.in;
}

