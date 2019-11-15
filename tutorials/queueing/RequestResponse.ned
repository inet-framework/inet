
package inet.tutorials.queueing;

import inet.queueing.sink.RequestConsumer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.source.ResponseProducer;

network RequestResponseNetwork
{
    @display("bgb=400,300");
    submodules:
        requestProducer: ActivePacketSource {
            @display("p=100,100");
        }
        responseConsumer: PassivePacketSink {
            @display("p=300,100");
        }
        requestConsumer: RequestConsumer {
            @display("p=100,200");
        }
        responseProducer: ResponseProducer {
            @display("p=300,200");
        }
    connections allowunconnected:
        requestProducer.out --> requestConsumer.in;
        responseProducer.out --> responseConsumer.in;
}
