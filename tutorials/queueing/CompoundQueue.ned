
package inet.tutorials.queueing;

import inet.queueing.classifier.WrrClassifier;
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
        classifier: WrrClassifier {
            @display("p=100,100");
        }
        queue1: PacketQueue {
            @display("p=325,100");
        }
        queue2: PacketQueue {
            @display("p=325,225");
        }
        scheduler: PriorityScheduler {
            @display("p=550,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        classifier.out++ --> queue1.in;
        classifier.out++ --> queue2.in;
        queue1.out --> scheduler.in++;
        queue2.out --> scheduler.in++;
        scheduler.out --> { @display("m=e"); } --> out;
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

