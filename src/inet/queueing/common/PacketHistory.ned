//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.contract.IPassivePacketSink;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketQueue;

//
// Packet history recorder that keeps copies of packets for inspection and debugging.
// Connects a packet producer to a consumer while maintaining a record of recent packets.
//
// Operation mechanism:
// - Receives packets via push operations from a connected producer
// - Forwards packets to the connected consumer without modification
// - Creates a copy of each packet and stores it in an internal queue
// - Maintains the last N packets (configurable via the size parameter)
// - Makes packet history available in the runtime user interface (Qtenv)
//
// This module is particularly useful for:
// - Debugging packet processing pipelines
// - Analyzing packet flows during simulation
// - Inspecting packet contents and metadata
// - Verifying correct packet handling in complex queueing networks
// - Creating packet traces for post-simulation analysis
//
// @see ~PacketQueue, ~PacketCloner
//
module PacketHistory like IPassivePacketSink, IActivePacketSource
{
    parameters:
        int size = default(100); // The number of packets to remember
        @display("i=block/passiveq");
    gates:
        input in @labels(push);
        output out @labels(push);
    submodules:
        cloner: PacketCloner {
            parameters:
                @display("p=100,100");
        }
        queue: <default("DropHeadQueue")> like IPacketQueue {
            parameters:
                packetCapacity = default(size);
                @display("p=100,200");
        }
    connections:
        in --> { @display("m=w"); } --> cloner.in;
        cloner.out++ --> { @display("m=e"); } --> out;
        cloner.out++ --> queue.in;
}
