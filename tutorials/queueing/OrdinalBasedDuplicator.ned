
package inet.tutorials.queueing;

import inet.queueing.common.OrdinalBasedDuplicator;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network OrdinalBasedDuplicatorNetwork
{
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        duplicator: OrdinalBasedDuplicator {
            @display("p=200,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
    connections:
        producer.out --> duplicator.in;
        duplicator.out --> consumer.in;
}
