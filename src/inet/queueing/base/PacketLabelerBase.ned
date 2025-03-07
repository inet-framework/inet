//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet labeler components in the queueing framework.
// Packet labelers attach labels to packets as they flow through the network,
// enabling classification, filtering, and special handling of packets.
//
// Labeling is a specific form of packet marking that attaches string-based
// identifiers to packets. These labels can be used by other components to:
// - Classify packets into different categories or flows
// - Filter packets based on their labels
// - Apply different processing rules to labeled packets
// - Implement quality of service mechanisms
// - Track and analyze specific packet flows
//
// Key features:
// - Supports a configurable list of labels to apply to packets
// - Preserves packet content while adding metadata
// - Operates transparently in packet processing pipelines
// - Derived modules can implement different labeling strategies
//
// Derived modules must implement specific labeling logic to determine
// which labels to attach to which packets based on packet attributes,
// content, or other criteria.
//
// @see ~PacketLabeler, ~ContentBasedLabeler, ~LabelClassifier, ~LabelFilter
//
simple PacketLabelerBase extends PacketMarkerBase
{
    parameters:
        string labels = default(""); // A space-separated list of labels that can be attached to packets
        @class(PacketLabelerBase);
        @display("i=block/star");
}
