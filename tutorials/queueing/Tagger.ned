
package inet.tutorials.queueing;

import inet.queueing.marker.PacketTagger;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network TaggerNetwork
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        tagger: PacketTagger {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections:
        producer.out --> tagger.in;
        tagger.out --> consumer.in;
}

