//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet classifier components in the queueing framework.
// Packet classifiers direct incoming packets to different outputs based on
// specific classification criteria, enabling traffic differentiation.
//
// Classifiers typically operate by:
// 1. Receiving packets pushed to their input
// 2. Examining packet properties based on classification criteria
// 3. Selecting an output gate based on the classification result
// 4. Pushing the packet to the selected output
//
// The classification decision can be based on various criteria:
// - Packet headers (IP addresses, port numbers, protocol types)
// - Packet content or payload
// - Packet metadata or attached tags
// - Traffic flow characteristics
// - Statistical properties or patterns
//
// Derived classifier modules implement various classification strategies including:
// - Priority-based classification
// - Content-based classification
// - Label-based classification
// - Flow-based classification
// - Token bucket and rate-based classification
//
// The reverseOrder parameter can be used to invert the priority order
// of outputs when using priority-based classification algorithms.
//
// @see ~IPacketClassifier, ~PacketClassifier, ~PriorityClassifier, ~LabelClassifier
//
simple PacketClassifierBase extends PacketProcessorBase
{
    parameters:
        bool reverseOrder = default(false); // When true, reverses the priority order of outputs for priority-based classifiers
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
