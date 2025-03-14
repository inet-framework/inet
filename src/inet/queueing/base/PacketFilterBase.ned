//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet filter components in the queueing framework.
// Packet filters selectively pass or drop packets based on specific
// criteria, enabling traffic control and policy enforcement.
//
// Filters typically operate by:
// 1. Receiving packets (either pushed to their input or pulled from their output)
// 2. Evaluating each packet against filter criteria
// 3. Either forwarding the packet or dropping it based on the evaluation
// 4. Maintaining statistics about passed and dropped packets
//
// The filtering decision can be based on various criteria:
// - Packet headers or content
// - Packet metadata or attached tags
// - Traffic patterns or rates
// - Statistical properties or random sampling
// - System state or external conditions
//
// The backpressure parameter determines how the filter behaves when
// packets are dropped in push mode:
// - When true, the filter signals back to prevent further packet pushes
// - When false, packets are silently dropped without backpressure
//
// Derived filter modules implement various filtering strategies including:
// - Content-based filtering
// - Label-based filtering
// - Statistical rate limiting
// - Random early detection
// - Ordinal-based dropping
//
// The module provides comprehensive statistics collection for:
// - Incoming, outgoing, and dropped packets
// - Packet lengths and data rates
// - Per-flow statistics for detailed traffic analysis
//
// @see ~IPacketFilter, ~PacketFilter, ~ContentBasedFilter, ~OrdinalBasedDropper
//
simple PacketFilterBase extends PacketProcessorBase
{
    parameters:
        bool backpressure = default(false); // When true, the filter applies backpressure when dropping packets in push mode
        displayStringTextFormat = default("dropped %d/%p pk (%k/%l)"); // Determines the text that is written on top of the submodule
        @class(PacketFilterBase);
        @display("i=block/downarrow");
        @signal[packetPushedIn](type=inet::Packet);
        @signal[packetPushedOut](type=inet::Packet);
        @signal[packetPulledIn](type=inet::Packet);
        @signal[packetPulledOut](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the incoming packet
        @statistic[incomingPackets](title="incoming packets"; source=merge(packetPushedIn, packetPulledIn); record=count; unit=pk);
        // the statistical value is the length of the incoming packet
        @statistic[incomingPacketLengths](title="incoming packet lengths"; source=packetLength(merge(packetPushedIn, packetPulledIn)); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the incoming packets
        @statistic[incomingDataRate](title="incoming data rate"; source=throughput(merge(packetPushedIn, packetPulledIn)); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the outgoing packet
        @statistic[outgoingPackets](title="outgoing packets"; source=merge(packetPushedOut, packetPulledOut); record=count; unit=pk);
        // the statistical value is the length of the outgoing packet
        @statistic[outgoingPacketLengths](title="outgoing packet lengths"; source=packetLength(merge(packetPushedOut, packetPulledOut)); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the outgoing packets
        @statistic[outgoingDataRate](title="outgoing data rate"; source=throughput(merge(packetPushedOut, packetPulledOut)); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the dropped packet
        @statistic[droppedPackets](title="dropped packets"; source=packetDropped; record=count; unit=pk);
        // the statistical value is the length of the dropped packet
        @statistic[droppedPacketLengths](title="dropped packet lengths"; source=packetLength(packetDropped); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the dropped packets
        @statistic[droppedDataRate](title="dropped data rate"; source=throughput(packetDropped); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the flow specific length of the incoming packet
        @statistic[flowIncomingPacketLengths](title="flow specific incoming packet lengths"; source=packetLength(demuxFlow(merge(packetPushedIn, packetPulledIn))); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the flow specific data rate of the incoming packets
        @statistic[flowIncomingDataRate](title="flow specific incoming data rate"; source=throughput(demuxFlow(merge(packetPushedIn, packetPulledIn))); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the flow specific length of the outgoing packet
        @statistic[flowOutgoingPacketLengths](title="flow specific outgoing packet lengths"; source=packetLength(demuxFlow(merge(packetPushedOut, packetPulledOut))); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the flow specific data rate of the outgoing packets
        @statistic[flowOutgoingDataRate](title="flow specific outgoing data rate"; source=throughput(demuxFlow(merge(packetPushedOut, packetPulledOut))); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the flow specific length of the dropped packet
        @statistic[flowDroppedPacketLengths](title="flow specific dropped packet lengths"; source=packetLength(demuxFlow(packetDropped)); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the flow specific data rate of the dropped packets
        @statistic[flowDroppedDataRate](title="flow specific dropped data rate"; source=throughput(demuxFlow(packetDropped)); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
}
