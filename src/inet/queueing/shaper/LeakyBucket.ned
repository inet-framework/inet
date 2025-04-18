//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.ITrafficConditioner;

//
// Implements a parameterizable leaky bucket algorithm.
//
module LeakyBucket like ITrafficConditioner
{
    parameters:
        @display("i=block/buffer");
    gates:
        input in @labels(push);
        output out @labels(pull);
    submodules:
        queue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=125,120");
        }
        server: <default("PacketServer")> like IPacketServer {
            parameters:
                @display("p=325,120");
        }
    connections:
        in --> { @display("m=w"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=e"); } --> out;
}

