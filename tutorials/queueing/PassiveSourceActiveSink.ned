
package inet.tutorials.queueing;

import inet.queueing.sink.ActivePacketSink;
import inet.queueing.source.PassivePacketSource;

network ProviderCollectorNetwork
{
    @display("bgb=400,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        collector: ActivePacketSink {
            @display("p=300,100");
        }
    connections:
        provider.out --> collector.in;
}

