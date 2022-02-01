//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketShaper;

//
// This module combines a packet queue and a packet gate into a packet shaper
// module. The queue stores the packets sorted according to its ordering and
// the gate decides when the first packet can be pulled from the queue.
//
module PacketShaper like IPacketShaper
{
    parameters:
        @display("i=block/bucket");
    gates:
        input in;
        output out;
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=100,150");
        }
        gate: <default("")> like IPacketGate {
            @display("p=300,150");
        }
    connections:
        in --> queue.in;
        queue.out --> gate.in;
        gate.out --> out;
}
