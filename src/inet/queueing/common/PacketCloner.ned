//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPassivePacketSink;

//
// Creates identical copies of each incoming packet and forwards them to multiple
// output gates. Serves as a packet distribution point where each outgoing connection
// receives the same packet content.
//
// Key features:
// - Creates exact copies of each incoming packet
// - Distributes copies to all connected output gates
// - Preserves all packet attributes and data in each copy
// - Operates in push mode (passive sink, active source)
// - Supports dynamic number of output connections
//
// Operation mechanism:
// - When a packet arrives at the input gate, the module creates a separate copy
//   for each output gate
// - Each copy is identical to the original packet, including all headers, data,
//   and metadata
// - The copies are then pushed to their respective output gates
// - The original packet is consumed in the process
//
// Applications:
// - Multicast packet distribution
// - Network traffic monitoring and analysis
// - Parallel packet processing pipelines
// - Redundant transmission paths
// - Testing multiple protocol implementations with identical traffic
// - Simulation of packet replication in broadcast networks
//
// Note: Unlike PacketDuplicator which sends duplicates to the same output,
// PacketCloner sends copies to different outputs, effectively implementing
// a one-to-many distribution pattern.
//
// @see ~PacketDuplicator, ~OrdinalBasedDuplicator, ~PacketMultiplexer
//
simple PacketCloner extends PacketProcessorBase like IPassivePacketSink // IActivePacketSource // TODO out gate is vector
{
    parameters:
        @class(PacketCloner);
        @display("i=block/star");
    gates:
        input in @labels(push);
        output out[] @labels(push);
}
