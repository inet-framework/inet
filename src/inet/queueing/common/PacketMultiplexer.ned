//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;

//
// Combines multiple packet streams into a single output stream in a push-based
// packet processing environment. Acts as a many-to-one adapter that allows
// multiple packet producers to share a single consumer.
//
// Key features:
// - Connects multiple packet producers to one packet consumer
// - Operates in push mode (passive sink, active source)
// - Maintains packet integrity during transfer
// - Supports dynamic number of input connections
// - Preserves packet order from each source (but interleaves between sources)
// - Optionally forwards service and protocol registrations
//
// Operation mechanism:
// - When any connected producer pushes a packet to one of the input gates,
//   the multiplexer immediately pushes it to the output gate
// - No packet transformation, filtering, or prioritization occurs
// - Packets are forwarded in the order they arrive from any input
// - If multiple producers push packets simultaneously, they are processed
//   in an implementation-defined order
//
// Note: Unlike PacketDemultiplexer which allows multiple consumers to access
// a single source, PacketMultiplexer combines multiple sources into one output.
// It is the push-based counterpart to the pull-based PacketScheduler.
//
// @see ~PacketDemultiplexer, ~PacketScheduler, ~PacketClassifier
//
simple PacketMultiplexer extends PacketProcessorBase like IActivePacketSource // TODO IPassivePacketSink // in gate is vector
{
    parameters:
        bool forwardServiceRegistration = default(true);
        bool forwardProtocolRegistration = default(true);
        displayStringTextFormat = default("passed %p pk (%l)"); // %p number of packets, %l total length
        @class(PacketMultiplexer);
        @display("i=-;b=10,10,oval,grey,,1");
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the multiplexed packet
        @statistic[packets](title="packets"; source=merge(packetPushed, packetPulled); record=count; unit=pk);
        // the statistical value is the length of the multiplexed packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(merge(packetPushed, packetPulled)); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the multiplexed packets
        @statistic[dataRate](title="data rate"; source=throughput(merge(packetPushed, packetPulled)); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in[] @labels(push);
        output out @labels(push);
}
