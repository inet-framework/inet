//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
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
