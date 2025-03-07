//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet shaper modules in the queueing framework.
// Packet shapers regulate traffic flow by controlling the timing
// of packet transmissions, without dropping or modifying packet
// contents.
//
// Key features:
// - Acts as both a passive packet sink and an active packet source
// - Controls packet transmission timing and rate
// - May delay packets to conform to traffic specifications
// - May reorder packets based on priority or other criteria
// - Never drops or modifies packet contents
// - Preserves overall packet count
// - Operates in push mode (passive sink, active source)
//
// @see ~PacketShaper, ~TokenBucket, ~LeakyBucket, ~PriorityShaper, ~TimeAwareShaper
//
moduleinterface IPacketShaper extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @display("i=block/bucket");
}
