
package inet.tutorials.queueing;

import inet.queueing.sink.ActivePacketSink;
import inet.queueing.queue.PacketQueue;
import inet.queueing.source.QueueFiller;

network QueueFillerNetwork
{
    @display("bgb=600,200");
    submodules:
        filler: QueueFiller {
            @display("p=100,100");
        }
        queue: PacketQueue {
            @display("p=300,100");
        }
        consumer: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        filler.out --> queue.in;
        queue.out --> consumer.in;
}
