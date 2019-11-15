
package inet.tutorials.queueing;

import inet.queueing.filter.OrdinalBasedDropper;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network OrdinalBasedDropperNetwork
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        dropper: OrdinalBasedDropper {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections:
        producer.out --> dropper.in;
        dropper.out --> consumer.in;
}
