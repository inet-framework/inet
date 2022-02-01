//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module classifies packets based on the attached labels in a ~LabelsTag.
//
simple LabelClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        int defaultGateIndex = default(0); // default gate index if no matching labels are found
        string labelsToGateIndices; // space separated list of label and gate index pairs
        @class(LabelClassifier);
}
