//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketLabelerBase;
import inet.queueing.contract.IPacketMarker;

//
// Deep packet inspection labeler that attaches identifying tags to packets based on
// their content. This module enables dynamic packet classification and processing
// by examining packet data and attaching appropriate metadata.
//
// Operation mechanism:
// - Evaluates each packet against a set of content-based filter expressions
// - For each matching filter, attaches the corresponding label to the packet
// - Multiple labels can be attached to a single packet if it matches multiple filters
// - Labels are stored in a ~LabelsTag that is attached to the packet
//
// The ContentBasedLabeler is particularly useful for:
// - Implementing deep packet inspection (DPI) systems
// - Creating content-aware traffic classification
// - Supporting policy-based routing based on packet content
// - Enabling advanced QoS mechanisms that consider packet payload
// - Building multi-criteria packet processing pipelines
//
// The module uses filter expressions that can examine any aspect of the packet,
// including headers, payload, and metadata, allowing for sophisticated packet
// classification based on actual content.
//
// @see ~PacketLabeler, ~LabelFilter, ~LabelClassifier, ~LabelScheduler, ~LabelsTag
//
simple ContentBasedLabeler extends PacketLabelerBase like IPacketMarker
{
    parameters:
        object packetFilters = default(["*"]); // Array of packet filter expressions, matches all packets by default
        @class(ContentBasedLabeler);
}
