//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet flow components in the queueing framework.
// Packet flow modules process packets as they pass through, without
// storing or delaying them.
//
// These modules support both push and pull packet processing modes:
// - In push mode, packets are pushed into the module's input and then
//   pushed out through its output after processing
// - In pull mode, packets are pulled from the module's input when they
//   are pulled from its output
//
// Packet flow modules are used to implement various packet processing
// functions that don't affect the timing or order of packets:
// - Packet filtering (accepting or dropping packets based on criteria)
// - Packet marking (attaching tags or modifying packet headers)
// - Packet metering (measuring packet rates and characteristics)
// - Packet classification (identifying packet types or flows)
//
// The module provides comprehensive statistics collection for:
// - Incoming and outgoing packet counts and lengths
// - Data rates for both incoming and outgoing traffic
// - Per-flow statistics for detailed traffic analysis
//
// @see ~IPacketFlow, ~PacketFilter, ~PacketMarker, ~PacketMeter, ~PacketTagger
//
simple PacketFlowBase extends PacketProcessorBase
{
    parameters:
        @class(PacketFlowBase);
        @display("i=block/rightarrow");
        @signal[packetPushedIn](type=inet::Packet);
        @signal[packetPushedOut](type=inet::Packet);
        @signal[packetPulledIn](type=inet::Packet);
        @signal[packetPulledOut](type=inet::Packet);
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
        // the statistical value is the flow specific length of the incoming packet
        @statistic[flowIncomingPacketLengths](title="flow specific incoming packet lengths"; source=packetLength(demuxFlow(merge(packetPushedIn, packetPulledIn))); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the flow specific data rate of the incoming packets
        @statistic[flowIncomingDataRate](title="flow specific incoming data rate"; source=throughput(demuxFlow(merge(packetPushedIn, packetPulledIn))); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the flow specific length of the outgoing packet
        @statistic[flowOutgoingPacketLengths](title="flow specific outgoing packet lengths"; source=packetLength(demuxFlow(merge(packetPushedOut, packetPulledOut))); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the flow specific data rate of the outgoing packets
        @statistic[flowOutgoingDataRate](title="flow specific outgoing data rate"; source=throughput(demuxFlow(merge(packetPushedOut, packetPulledOut))); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
}
