
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.scheduler.MarkovScheduler;
import inet.queueing.source.ActivePacketSource;

network Burst1Network
{
    @display("bgb=600,300");
    submodules:
        producer1: ActivePacketSource {
            @display("p=100,100");
        }
        producer2: ActivePacketSource {
            @display("p=100,200");
        }
        scheduler: MarkovScheduler {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer1.out --> scheduler.in++;
        producer2.out --> scheduler.in++;
        scheduler.out --> consumer.in;
}

