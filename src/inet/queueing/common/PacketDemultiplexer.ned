//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSink;

//
// Enables multiple consumers to pull packets from a single provider in a
// pull-based packet processing environment. Acts as a many-to-one adapter
// that allows multiple packet collectors to share a single packet source.
//
// Key features:
// - Connects multiple packet collectors to one packet provider
// - Operates in pull mode (active sink, passive source)
// - Maintains packet integrity during transfer
// - Supports dynamic number of output connections
// - Preserves packet order from the source
//
// Operation mechanism:
// - When any connected consumer pulls a packet from one of the output gates,
//   the demultiplexer pulls a packet from its input gate
// - The pulled packet is then forwarded to the requesting consumer
// - No packet transformation or filtering occurs during this process
// - If multiple consumers request packets simultaneously, they are served
//   in an implementation-defined order
//
// Applications:
// - Sharing a single packet source among multiple consumers
// - Implementing load distribution across multiple processing paths
// - Creating fan-out topologies in pull-based packet processing networks
// - Connecting a single queue to multiple packet processors
// - Building flexible packet processing pipelines
//
// Note: Unlike PacketMultiplexer which combines multiple sources into one output,
// PacketDemultiplexer allows multiple consumers to access a single source.
// It is the pull-based counterpart to the push-based PacketCloner.
//
// @see ~PacketMultiplexer, ~PacketCloner, ~PacketScheduler
//
simple PacketDemultiplexer extends PacketProcessorBase like IActivePacketSink // TODO , IPassivePacketSource // out gate is vector
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        @class(PacketDemultiplexer);
        @display("i=block/fork");
    gates:
        input in @labels(pull);
        output out[] @labels(pull);
}
