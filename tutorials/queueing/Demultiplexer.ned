
package inet.tutorials.queueing;

import inet.queueing.sink.ActivePacketSink;
import inet.queueing.common.PacketDemultiplexer;
import inet.queueing.source.PassivePacketSource;

network DemultiplexerNetwork
{
    parameters:
//    	@display("bgb=600");
        int numCollectors;
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        demultiplexer: PacketDemultiplexer {
            @display("p=300,100");
        }
        collector[numCollectors]: ActivePacketSink {
            @display("p=500,100,c,100");
        }
    connections allowunconnected:
        provider.out --> demultiplexer.in;
        for i=0..numCollectors-1 {
            demultiplexer.out++ --> collector[i].in;
        }
}

