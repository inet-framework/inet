
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network ProducerConsumerNetwork
{
    @display("bgb=400,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
    connections:
        producer.out --> consumer.in;
}

