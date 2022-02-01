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
// This module connects one packet producer to one packet consumer. It can be
// pushed with packets from the connected packet producer. It keeps a copy of
// the last N packets pushed into its input. The packets are available in the
// runtime user interface (Qtenv) for inspection.
//
module PacketHistory like IPassivePacketSink, IActivePacketSource
{
    parameters:
        int size = default(100); // the number of packets to remember
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

