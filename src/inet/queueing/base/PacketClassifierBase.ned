//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet classifier modules. Derived modules
// must implement a single packet classifier function which determines the index
// of the output gate for the next pushed packet.
//
// @see ~IPacketClassifier
//
simple PacketClassifierBase extends PacketProcessorBase
{
    parameters:
        bool reverseOrder = default(false);
        displayStringTextFormat = default("classified %p pk (%l)");
        @class(PacketClassifierBase);
        @display("i=block/classifier");
        @signal[packetPushed](type=inet::Packet);
        // the statistical value is the classified packet
        @statistic[packets](title="packets"; source=packetPushed; record=count; unit=pk);
        // the statistical value is the length of the classified packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPushed); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the classified packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPushed); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in;
        output out[];
}
