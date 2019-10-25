
package inet.tutorials.queueing;

import inet.queueing.classifier.PacketClassifier;
import inet.queueing.queue.CompoundPacketQueue;
import inet.queueing.queue.PacketQueue;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.scheduler.PriorityScheduler;

module TestCompoundPacketQueue extends CompoundPacketQueue
{
    parameters:
        @class(::inet::queueing::CompoundPacketQueue);
    submodules:
        classifier: PacketClassifier {
            @display("p=100,100");
        }
        queue1: PacketQueue {
            @display("p=200,100");
        }
        queue2: PacketQueue {
            @display("p=200,200");
        }
        scheduler: PriorityScheduler {
            @display("p=300,100");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> queue1.in;
        classifier.out++ --> queue2.in;
        queue1.out --> scheduler.in++;
        queue2.out --> scheduler.in++;
        scheduler.out --> out;
}

network CompoundPacketQueueNetwork
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        queue: TestCompoundPacketQueue {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> queue.in;
        queue.out --> collector.in;
}

