//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Deep packet inspection classifier that selects output gates based on packet
// content using configurable filter expressions. This enables sophisticated traffic
// management based on packet data and metadata.
//
// Operation mechanism:
// - Evaluates each packet against the configured filter expressions in order
// - The first matching filter expression determines which output gate to use
// - If no filter matches, the defaultGateIndex is used
// - Filter expressions can examine any aspect of the packet, including headers,
//   payload, and attached tags
//
// The ContentBasedClassifier is particularly useful for:
// - Deep packet inspection and content-based routing
// - Protocol-specific traffic management
// - Implementing complex Quality of Service policies
// - Security filtering and traffic segregation
// - Application-layer traffic classification
//
// The packetFilters parameter contains an array of filter expressions that are
// evaluated in order, with the first match determining the output gate to use.
//
// @see ~PacketClassifier, ~LabelClassifier, ~PriorityClassifier
//
simple ContentBasedClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        int defaultGateIndex = default(0); // Default gate index if no matching labels are found
        object packetFilters; // Array of packet filter expressions
        @class(ContentBasedClassifier);
}
