//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Extensible packet classifier that uses a pluggable classification function to
// determine which output gate to forward packets to. This provides a framework
// for implementing custom packet classification algorithms.
//
// Operation mechanism:
// - Receives packets from a single input (producer)
// - Invokes the configured classifier function for each packet
// - The function examines the packet and returns an integer
// - This integer determines which output gate the packet is forwarded to
//
// The PacketClassifier serves as a base for implementing custom classification
// algorithms without having to implement a complete classifier module. By providing
// different classifier function implementations, various classification policies
// can be realized.
//
// This classifier is particularly useful for:
// - Traffic differentiation based on packet attributes
// - Quality of Service implementations
// - Protocol-specific routing of packets
// - Load balancing across multiple processing paths
// - Implementing experimental classification algorithms
//
// The classifierClass parameter specifies the C++ class that implements the
// classification algorithm. This class must implement the IPacketClassifierFunction
// interface and be registered via Register_Class.
//
// @see ~PriorityClassifier, ~ContentBasedClassifier, ~LabelClassifier, ~MarkovClassifier
//
simple PacketClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string classifierClass; // Determines the output queue of packets, the parameter must be the name of a C++ class which implements the IPacketClassifierFunction interface and is registered via Register_Class
        @class(PacketClassifier);
}
