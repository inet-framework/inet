//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.contract.IPacketDelayer;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;

//
// Compound module that delays packets for a specified amount of time while
// preserving their order. Implements the IPacketDelayer interface using a queue
// and server architecture. The queue stores packets while they wait, and the
// server processes each packet with a processing time equal to the configured
// delay. Note that the actual packet delay will be the sum of the queueing time
// and the configured delay. Unlike the simple PacketDelayer module, this 
// implementation always maintains packet order regardless of delay distribution.
//
module QueueingPacketDelayer like IPacketDelayer
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        volatile double delay @unit(s);
        *.clockModule = default(absPath(this.clockModule));
        @display("i=block/delay");
    gates:
        input in @labels(push);
        output out @labels(push);
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            parameters:
                @display("p=150,100");
        }
        server: <default("PacketServer")> like IPacketServer {
            parameters:
                processingTime = default(parent.delay);
                @display("p=350,100");
        }
    connections:
        in --> { @display("m=w"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=e"); } --> out;
}

