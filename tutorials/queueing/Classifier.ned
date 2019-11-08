
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.classifier.WrrClassifier;
import inet.queueing.source.ActivePacketSource;

network ClassifierNetwork
{
    @display("bgb=600,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: WrrClassifier {
            @display("p=300,100");
        }
        consumer1: PassivePacketSink {
            @display("p=500,100");
        }
        consumer2: PassivePacketSink {
            @display("p=500,200");
        }
    connections allowunconnected:
        producer.out --> classifier.in;
        classifier.out++ --> consumer1.in;
        classifier.out++ --> consumer2.in;
}

