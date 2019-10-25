
package inet.tutorials.queueing;

import inet.queueing.sink.ActivePacketSink;
import inet.queueing.queue.PacketQueue;
import inet.queueing.scheduler.PriorityScheduler;
import inet.queueing.source.ActivePacketSource;

network PrioritySchedulerNetwork
{
    @display("bgb=800,300");
    submodules:
        producer1: ActivePacketSource {
            @display("p=100,100");
        }
        producer2: ActivePacketSource {
            @display("p=100,225");
        }
        queue1: PacketQueue {
            @display("p=300,100");
        }
        queue2: PacketQueue {
            @display("p=300,225");
        }
        scheduler: PriorityScheduler {
            @display("p=500,100");
        }
        collector: ActivePacketSink {
            @display("p=700,100");
        }
    connections allowunconnected:
        producer1.out --> queue1.in;
        producer2.out --> queue2.in;
        queue1.out --> scheduler.in++;
        queue2.out --> scheduler.in++;
        scheduler.out --> collector.in;
}
