
package inet.tutorials.queueing;

import inet.queueing.classifier.WrrClassifier;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.PacketQueue;
import inet.queueing.scheduler.PriorityScheduler;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.source.ActivePacketSource;


network RedDropperNetwork
{
    @display("bgb=1250,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: WrrClassifier {
            @display("p=300,100");
        }
        dropper1: RedDropper {
            @display("p=500,100");
        }
        dropper2: RedDropper {
            @display("p=500,225");
        }
        queue1: PacketQueue {
            @display("p=725,100");
        }
        queue2: PacketQueue {
            @display("p=725,225");
        }
        scheduler: PriorityScheduler {
            @display("p=950,100");
        }
        collector: ActivePacketSink {
            @display("p=1150,100");
        }
    connections:
        producer.out --> classifier.in;
        classifier.out++ --> dropper1.in;
        classifier.out++ --> dropper2.in;
        dropper1.out --> queue1.in;
        dropper2.out --> queue2.in;
        queue1.out --> scheduler.in++;
        queue2.out --> scheduler.in++;
        scheduler.out --> collector.in;
}
