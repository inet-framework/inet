//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet server modules.
//
// @see ~IPacketServer
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
