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
// Compound packet delayer that introduces configurable delay to packets
// using a combination of queue and server modules. Provides more realistic
// delay behavior than simple timers.
//
// Operation mechanism:
// - Receives packets through its input gate
// - Stores incoming packets in an internal queue
// - Processes packets one by one using a server module
// - Server holds each packet for the configured delay time
// - Outputs packets after the delay period has elapsed
// - Supports external clock module for synchronized timing
//
// The compound structure with separate queue and server components allows
// for more realistic behavior than a simple timer-based delayer, especially
// under high load conditions.
//
// @see ~PacketDelayer, ~PacketServer, ~PacketQueue
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
