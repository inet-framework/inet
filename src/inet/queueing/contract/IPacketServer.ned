//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Module interface for packet server components in the queueing framework.
// Packet servers process packets with a configurable service time, modeling
// the time needed to process or transmit a packet.
//
// Key characteristics:
// - Has one active input and one active output
// - Actively pulls packets from its input
// - Actively pushes packets to its output
// - Can introduce a processing delay for each packet
// - Preserves packet order (FIFO processing)
// - Controls the rate at which packets flow through the system
//
// Packet servers are commonly used in network devices to model:
// - Transmission delays in network interfaces
// - Processing delays in routers and switches
// - Service times in servers and processing nodes
// - Rate limiting and traffic shaping mechanisms
// - CPU or other resource constraints
//
// Common implementations include PacketServer, InstantServer, PreemptingServer,
// and TokenBasedServer. When a server is not needed in a model, OmittedPacketServer
// can be used as a placeholder.
//
moduleinterface IPacketServer extends IActivePacketSink, IActivePacketSource
{
    parameters:
        @omittedTypename(OmittedPacketServer);
        @display("i=block/server");
}
