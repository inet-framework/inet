//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Tag-based packet classifier that routes packets based on attached labels.
// This module enables classification of pre-tagged packets without
// deep packet inspection.
//
// Operation mechanism:
// - Examines the LabelsTag attached to each packet
// - Maps labels to output gate indices using the labelsToGateIndices parameter
// - If a packet has multiple labels, the first matching label determines the output
// - If no labels match or the packet has no LabelsTag, uses the defaultGateIndex
//
// @see ~ContentBasedClassifier
//
simple LabelClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        int defaultGateIndex = default(0); // Default gate index if no matching labels are found
        string labelsToGateIndices; // Space separated list of label and gate index pairs
        @class(LabelClassifier);
}
