
package inet.tutorials.queueing;

import inet.queueing.buffer.PriorityBuffer;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.queue.CompoundPacketQueue;
import inet.queueing.queue.PacketQueue;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.scheduler.PriorityScheduler;

module TestQueue extends CompoundPacketQueue
{
    parameters:
        *.bufferModule = "^.buffer";
        @class(::inet::queueing::CompoundPacketQueue);
    submodules:
        buffer: PriorityBuffer {
            @display("p=100,100");
        }
        classifier: PacketClassifier {
            @display("p=200,100");
        }
        queue1: PacketQueue {
            @display("p=300,100");
        }
        queue2: PacketQueue {
            @display("p=300,200");
        }
        scheduler: PriorityScheduler {
            @display("p=400,100");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> queue1.in;
        classifier.out++ --> queue2.in;
        queue1.out --> scheduler.in++;
        queue2.out --> scheduler.in++;
        scheduler.out --> out;
}

network BufferNetwork
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        queue: TestQueue {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> queue.in;
        queue.out --> collector.in;
}

