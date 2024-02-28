//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.contract.IPacketDelayer;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;

module QueueingPacketDelayer like IPacketDelayer
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        volatile double delay @unit(s);
        *.clockModule = default(absPath("^" + clockModule));
        @class(PacketDelayer);
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
                processingTime = default(delay);
                @display("p=350,100");
        }
    connections:
        in --> { @display("m=w"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=e"); } --> out;
}

