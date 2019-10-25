
package inet.tutorials.queueing;

import inet.queueing.sink.ActivePacketSink;
import inet.queueing.queue.PacketQueue;
import inet.queueing.classifier.PriorityClassifier;
import inet.queueing.source.ActivePacketSource;

network PriorityClassifierNetwork
{
    @display("bgb=800,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: PriorityClassifier {
            @display("p=300,100");
        }
        queue1: PacketQueue {
            @display("p=500,100");
        }
        queue2: PacketQueue {
            @display("p=500,225");
        }
        collector1: ActivePacketSink {
            @display("p=700,100");
        }
        collector2: ActivePacketSink {
            @display("p=700,225");
        }
    connections allowunconnected:
        producer.out --> classifier.in;
        classifier.out++ --> queue1.in;
        classifier.out++ --> queue2.in;
        queue1.out --> collector1.in;
        queue2.out --> collector2.in;
}
