//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet server components in the queueing framework.
// Packet servers process packets for a specified amount of time before
// forwarding them, modeling service time in queueing systems.
//
// Servers typically operate by:
// 1. Pulling a packet from their input (usually a queue)
// 2. Processing the packet for a specified service time
// 3. Pushing the packet to their output when processing completes
//
// The service time can be:
// - Constant (fixed processing time)
// - Random (following statistical distributions)
// - Data-dependent (based on packet size)
// - State-dependent (based on system load)
//
// Derived server modules implement various service disciplines including:
// - Standard servers with configurable service time
// - Preempting servers that can interrupt processing
// - Token-based servers with rate limiting
// - Specialized servers for specific protocols
//
// The module provides comprehensive statistics collection for:
// - Number of packets processed
// - Packet lengths and data rates
// - Service time measurements
//
// @see ~IPacketServer, ~PacketServer, ~PreemptingServer, ~TokenBasedServer
//
simple PacketServerBase extends PacketProcessorBase
{
    parameters:
        displayStringTextFormat = default("served %p pk (%l)");
        @class(PacketServerBase);
        @display("i=block/server");
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the incoming packet
        @statistic[incomingPackets](title="incoming packets"; source=packetPulled; record=count; unit=pk);
        // the statistical value is the length of the incoming packet
        @statistic[incomingPacketLengths](title="incoming packet lengths"; source=packetLength(packetPulled); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the incoming packets
        @statistic[incomingDataRate](title="incoming datarate"; source=throughput(packetPulled); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the outgoing packet
        @statistic[outgoingPackets](title="outgoing packets"; source=packetPushed; record=count; unit=pk);
        // the statistical value is the length of the outgoing packet
        @statistic[outgoingPacketLengths](title="outgoing packet lengths"; source=packetLength(packetPushed); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the outgoing packets
        @statistic[outgoingDataRate](title="outgoing datarate"; source=throughput(packetPushed); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in;
        output out;
}
