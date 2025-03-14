//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Module interface for packet meter components in the queueing framework.
// Packet meters measure traffic characteristics and attach this information
// to packets as they flow through the network.
//
// Key characteristics:
// - Has one input and one output
// - Can operate in both push and pull modes
// - Processes packets as part of a stream (considers packet relationships)
// - Measures traffic properties like rate, burst size, or conformance
// - Attaches measurement results as metadata to packets
// - Does not delay, reorder, or drop packets
//
// Packet meters are commonly used in network devices to:
// - Implement traffic policing and shaping mechanisms
// - Support quality of service (QoS) policies
// - Measure conformance to traffic contracts
// - Classify traffic into different service categories
// - Provide input for congestion control algorithms
//
// Common implementations include TokenBucketMeter, which classifies packets
// as conforming or non-conforming based on a token bucket algorithm. When a
// meter is not needed in a model, OmittedPacketMeter can be used as a placeholder.
//
moduleinterface IPacketMeter extends IPacketFlow
{
    parameters:
        @omittedTypename(OmittedPacketMeter);
        @display("i=block/star");
}
