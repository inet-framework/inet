//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Tag-based packet filter that selectively forwards packets based on attached labels.
// This module enables efficient filtering of pre-tagged packets without requiring
// deep packet inspection.
//
// Operation mechanism:
// - Examines the LabelsTag attached to each packet
// - Compares packet labels against the configured labelFilter expression
// - Forwards packets with matching labels, drops non-matching packets
// - In push mode: Processes packets from producer, forwards matching ones to consumer
// - In pull mode: Pulls packets from provider until finding one with matching labels
//
// The LabelFilter is particularly useful for:
// - Implementing MPLS-like label-based forwarding
// - Efficient filtering of pre-processed packets
// - Multi-stage filtering pipelines
// - Service differentiation based on application-level tagging
// - Implementing policy-based routing with minimal overhead
//
// The labelFilter parameter supports complex expressions with wildcards and
// Boolean operators (AND, OR, NOT), allowing for sophisticated filtering rules
// based on packet labels.
//
// @see ~PacketFilter, ~ContentBasedFilter, ~LabelClassifier
//
simple LabelFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string labelFilter @mutable; // The label filter as a match expression (e.g. green or red)
        @class(LabelFilter);
}
