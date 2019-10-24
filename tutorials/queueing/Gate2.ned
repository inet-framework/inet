
package inet.tutorials.queueing;

import inet.queueing.common.PacketGate;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.source.PassivePacketSource;

network Gate2Network
{
    @display("bgb=600,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        gate: PacketGate {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections:
        provider.out --> gate.in;
        gate.out --> collector.in;
}

