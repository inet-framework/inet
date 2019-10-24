
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.PassivePacketSource;
import inet.queueing.server.PacketServer;

network ServerNetwork
{
    @display("bgb=600,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        server: PacketServer {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider.out --> server.in;
        server.out --> consumer.in;
}
