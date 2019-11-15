
package inet.tutorials.queueing;

import inet.queueing.source.PassivePacketSource;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.queue.PacketQueue;
import inet.queueing.tokengenerator.QueueBasedTokenGenerator;
import inet.queueing.server.TokenBasedServer;

network InterfaceNetwork
{
    @display("bgb=400,575");
    submodules:
        provider: PassivePacketSource {
            @display("p=125,100");
        }
        server: TokenBasedServer {
            @display("p=125,225");
        }
        queue: PacketQueue {
            @display("p=125,350");
        }
        collector: ActivePacketSink {
            @display("p=125,475");
        }
        tokenGenerator: QueueBasedTokenGenerator {
            @display("p=300,100");
        }
    connections allowunconnected:
        provider.out --> server.in;
        server.out --> queue.in;
        queue.out --> collector.in;
}
