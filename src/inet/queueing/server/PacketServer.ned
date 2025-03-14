//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// Packet server that models packet processing with configurable processing time
// and bitrate. This module introduces a delay between receiving and forwarding
// packets, representing processing overhead in network elements.
//
// Operation mechanism:
// - Pulls packets from a connected packet provider (e.g., a queue)
// - Holds each packet for a calculated processing delay
// - Pushes the processed packet to a connected packet consumer
// - Processing delay is calculated as: processingTime + packetLength / processingBitrate
//
// The PacketServer is commonly used to model:
// - Processing delay in network devices (routers, switches)
// - Service time in queueing systems
// - CPU processing time in network protocol stacks
// - Transmission delay in communication channels
// - Any network element with non-zero processing overhead
//
// The processingTime parameter specifies a fixed processing delay per packet,
// while processingBitrate introduces a length-dependent component to model
// processing that scales with packet size.
//
// @see ~InstantServer, ~PreemptingServer, ~TokenBasedServer
//
simple PacketServer extends PacketServerBase like IPacketServer
{
    parameters:
        displayStringTextFormat = default("served %p pk (%l)\n%s");
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        int serveSchedulingPriority = default(-1); // Specifies the FES scheduling priority for the extra event that is pulling the packet, -1 means no extra event
        volatile double processingTime @unit(s); // Determines additional processing time per packet
        volatile double processingBitrate @unit(bps) = default(inf bps); // Determines additional processing time per bit
        @class(PacketServer);
}
