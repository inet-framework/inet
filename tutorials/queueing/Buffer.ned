
package inet.tutorials.queueing;

import inet.queueing.buffer.PacketBuffer;
import inet.queueing.queue.PacketQueue;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.source.ActivePacketSource;


network BufferNetwork
{
    submodules:
        buffer: PacketBuffer {
            @display("p=125,350");
        }
        producer1: ActivePacketSource {
            @display("p=125,100");
        }
        producer2: ActivePacketSource {
            @display("p=125,225");
        }
        queue1: PacketQueue {
            @display("p=325,100");
        }
        queue2: PacketQueue {
            @display("p=325,225");
        }
        collector1: ActivePacketSink {
            @display("p=525,100");
        }
        collector2: ActivePacketSink {
            @display("p=525,225");
        }
    connections:
        producer1.out --> queue1.in;
        queue1.out --> collector1.in;
        producer2.out --> queue2.in;
        queue2.out --> collector2.in;
}

