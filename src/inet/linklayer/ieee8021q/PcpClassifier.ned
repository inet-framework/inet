//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module classifies packets based on the attached PCP value [0, 7]. The
// PCP is determined by a PcpReq or a PcpInd or both. The output gate index is
// the ith value in the pcpToGateIndex parameter.
//
simple PcpClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        object pcpToGateIndex; // specifies the gate index for all PCP values in ascending order
        int defaultGateIndex = default(0); // default gate index if no PCP found
        @class(PcpClassifier);
}
