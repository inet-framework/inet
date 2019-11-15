
package inet.tutorials.queueing;

import inet.queueing.classifier.LabelClassifier;
import inet.queueing.marker.ContentBasedLabeler;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;


network LabelerNetwork
{
    @display("bgb=800,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        labeler: ContentBasedLabeler {
            @display("p=300,100");
        }
        classifier: LabelClassifier {
            @display("p=500,100");
        }
        consumer1: PassivePacketSink {
            @display("p=700,100");
        }
        consumer2: PassivePacketSink {
            @display("p=700,200");
        }
    connections allowunconnected:
        producer.out --> labeler.in;
        labeler.out --> classifier.in;
        classifier.out++ --> consumer1.in;
        classifier.out++ --> consumer2.in;
}
