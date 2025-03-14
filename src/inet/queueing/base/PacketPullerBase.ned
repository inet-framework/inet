//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet puller components in the queueing framework.
// Packet pullers actively request and pull packets from connected passive
// packet sources, controlling the flow of packets through the network.
//
// Packet pullers operate in pull mode, where they initiate the packet
// transfer by requesting packets from their sources. This is in contrast
// to push mode, where sources actively push packets to their destinations.
//
// Key features:
// - Actively controls when packets are requested from sources
// - Determines the timing and rate of packet processing
// - Provides the foundation for implementing various packet collection strategies
// - Can implement traffic shaping, rate limiting, and controlled packet processing
//
// Packet pullers are used to implement:
// - Active packet sinks that collect packets at specific rates
// - Packet servers that process packets with service time delays
// - Rate controllers that limit traffic to specific bandwidths
// - Schedulers that pull packets from multiple sources based on priorities
//
// Derived modules must implement specific pulling strategies by defining
// when to request packets from sources and how to process them.
//
// @see ~PacketPuller, ~ActivePacketSink, ~PacketServer
//
simple PacketPullerBase extends PacketProcessorBase
{
    parameters:
        @class(PacketPullerBase);
    gates:
        input in;  // Input gate connected to a passive packet source
        output out; // Output gate for forwarding pulled packets
}
