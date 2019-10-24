
package inet.tutorials.queueing;

import inet.queueing.sink.ActivePacketSink;
import inet.queueing.classifier.MarkovClassifier;
import inet.queueing.source.PassivePacketSource;

network Burst2Network
{
    @display("bgb=600,300");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        classifier: MarkovClassifier {
            @display("p=300,100");
        }
        collector1: ActivePacketSink {
            @display("p=500,100");
        }
        collector2: ActivePacketSink {
            @display("p=500,200");
        }
    connections allowunconnected:
        provider.out --> classifier.in;
        classifier.out++ --> collector1.in;
        classifier.out++ --> collector2.in;
}

