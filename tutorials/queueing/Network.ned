
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.common.PacketDelayer;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.PacketServer;

module TestMac
{
    @display("bgb=400,300");
    gates:
        input lowerIn;
        input upperIn;
        output lowerOut;
        output upperOut;
    submodules:
        queue: PacketQueue {
            @display("p=200,100");
        }
        server: PacketServer {
            @display("p=200,225");
        }
    connections:
        upperIn --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=s"); } --> lowerOut;
        lowerIn --> { @display("m=m,66,100,66,0"); } --> upperOut;
}

module TestHost
{
    gates:
        input lowerIn;
        output lowerOut;
    submodules:
        sourceApplication: ActivePacketSource {
            @display("p=100,100");
        }
        destinationApplication: PassivePacketSink {
            @display("p=300,100");
        }
        mac: TestMac {
            @display("p=200,200");
        }
    connections:
        sourceApplication.out --> mac.upperIn;
        mac.lowerOut --> lowerOut;
        lowerIn --> mac.lowerIn;
        mac.upperOut --> destinationApplication.in;
}

module TestCable
{
    gates:
        input aIn;
        output aOut;
        input bIn;
        output bOut;
    submodules:
        aDelayer: PacketDelayer {
            @display("p=100,100");
        }
        bDelayer: PacketDelayer {
            @display("p=100,200");
        }
    connections:
        aIn --> { @display("m=w"); } --> aDelayer.in;
        aDelayer.out --> { @display("m=e"); } --> aOut;
        bIn --> { @display("m=w"); } --> bDelayer.in;
        bDelayer.out --> { @display("m=e"); } --> bOut;
}

network TestNetwork
{
    submodules:
        source: TestHost {
            @display("p=100,100");
        }
        cable: TestCable {
            @display("p=200,100");
        }
        destination: TestHost {
            @display("p=300,100");
        }
    connections:
        source.lowerOut --> cable.aIn;
        cable.aOut --> destination.lowerIn;
        destination.lowerOut --> cable.bIn;
        cable.bOut --> source.lowerIn;
}
