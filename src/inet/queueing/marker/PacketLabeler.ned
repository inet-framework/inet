//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketMarkerBase;
import inet.queueing.contract.IPacketMarker;

//
// Packet labeling module that attaches identifying tags to packets based on configurable
// filter criteria. This module enables dynamic packet classification and processing
// by attaching metadata that can be used by downstream modules.
//
// Operation mechanism:
// - Evaluates each packet against a set of filter functions
// - For each matching filter, attaches the corresponding label to the packet
// - Multiple labels can be attached to a single packet if it matches multiple filters
// - Labels are stored in a ~LabelsTag that is attached to the packet
//
// The module uses pluggable filter functions that implement the IPacketFilterFunction
// interface, allowing for flexible and extensible packet classification criteria.
//
// @see ~LabelFilter, ~LabelClassifier, ~LabelScheduler, ~LabelsTag
//
simple PacketLabeler extends PacketMarkerBase like IPacketMarker
{
    parameters:
        string filterClasses = default(""); // Determines the packets that are marked. The parameter must be the name of a C++ class that implements the IPacketFilterFunction interface and is registered via Register_Class.
        string labels = default(""); // A space-separated list of labels.
        @display("i=block/star");
        @class(PacketLabeler);
}
