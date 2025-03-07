//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet pusher components in the queueing framework.
// Packet pushers actively send packets to connected passive packet
// sinks, controlling the flow of packets through the network.
//
// Packet pushers operate in push mode, where they initiate the packet
// transfer by sending packets to their destinations. This is in contrast
// to pull mode, where destinations actively request packets from sources.
//
// Key features:
// - Actively controls when packets are sent to destinations
// - Determines the timing and rate of packet processing
// - Provides the foundation for implementing various packet generation strategies
// - Can implement traffic generation, rate control, and burst patterns
//
// Packet pushers are used to implement:
// - Active packet sources that generate packets at specific rates
// - Traffic generators with configurable patterns
// - Packet forwarders that control the timing of packet delivery
// - Burst generators that create traffic spikes
//
// Derived modules must implement specific pushing strategies by defining
// when to generate or forward packets to destinations and how to process them.
//
// @see ~PacketPusher, ~ActivePacketSource, ~PacketProducer
//
simple PacketPusherBase extends PacketProcessorBase
{
    parameters:
        @class(PacketPusherBase);
    gates:
        input in;  // Input gate for receiving packets to push
        output out; // Output gate connected to a passive packet sink
}
