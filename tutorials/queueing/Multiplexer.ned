
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.source.ActivePacketSource;

network MultiplexerNetwork
{
    parameters:
        int numProducers;
    submodules:
        producer[numProducers]: ActivePacketSource {
            @display("p=100,100,c,100");
        }
        multiplexer: PacketMultiplexer {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        for i=0..numProducers-1 {
            producer[i].out --> multiplexer.in++;
        }
        multiplexer.out --> consumer.in;
}
