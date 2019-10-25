
package inet.tutorials.queueing;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.common.PacketDelayer;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.PacketServer;

module TestMac
{
    gates:
        input lowerIn;
        input upperIn;
        output lowerOut;
        output upperOut;
    submodules:
        queue: PacketQueue;
        server: PacketServer;
    connections:
        upperIn --> queue.in;
        queue.out --> server.in;
        server.out --> lowerOut;
        lowerIn --> upperOut;
}

module TestHost
{
    gates:
        input lowerIn;
        output lowerOut;
    submodules:
        sourceApplication: ActivePacketSource;
        destinationApplication: PassivePacketSink;
        mac: TestMac;
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
        aDelayer: PacketDelayer;
        bDelayer: PacketDelayer;
    connections:
        aIn --> aDelayer.in;
        aDelayer.out --> aOut;
        bIn --> bDelayer.in;
        bDelayer.out --> bOut;
}

network TestNetwork
{
    submodules:
        source: TestHost;
        destination: TestHost;
        cable: TestCable;
    connections:
        source.lowerOut --> cable.aIn;
        cable.aOut --> destination.lowerIn;
        destination.lowerOut --> cable.bIn;
        cable.bOut --> source.lowerIn;
}
