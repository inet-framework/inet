//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.tsn.trafficshaping.underthehood;

import inet.protocolelement.shaper.EligibilityTimeFilter;
import inet.protocolelement.shaper.EligibilityTimeGate;
import inet.protocolelement.shaper.EligibilityTimeMeter;
import inet.protocolelement.shaper.EligibilityTimeQueue;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.server.PacketServer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

network PeekingUnderTheHoodShowcase
{
    parameters:
        int numSources;
    submodules:
        source[numSources]: ActivePacketSource {
            @display("p=100,100,col,150");
        }
        meter[numSources]: EligibilityTimeMeter {
            @display("p=300,100,col,150");
        }
        multiplexer: PacketMultiplexer {
            @display("p=500,100");
        }
        filter: EligibilityTimeFilter {
            @display("p=700,100");
        }
        queue: EligibilityTimeQueue {
            @display("p=900,100");
        }
        gate: EligibilityTimeGate {
            @display("p=1100,100");
        }
        server: PacketServer {
            @display("p=1300,100");
        }
        sink: PassivePacketSink {
            @display("p=1500,100");
        }
    connections:
        for i=0..numSources-1 {
            source[i].out --> meter[i].in;
            meter[i].out --> multiplexer.in++;
        }
        multiplexer.out --> filter.in;
        filter.out --> queue.in;
        queue.out --> gate.in;
        gate.out --> server.in;
        server.out --> sink.in;
}
