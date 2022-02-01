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
// the value found in the mapping matrix using the PCP value as the row index
// and the number of connected consumers (traffic categories) as the column
// index.
//
module PcpTrafficClassClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        // IEEE Standard for Local and Metropolitan Area Networks, Bridges and Bridged Networks
        // Table 8-5—Recommended priority to traffic class mappings
        object mapping = default([[0, 0, 0, 0, 0, 1, 1, 1],   // BE best effort
                                  [0, 0, 0, 0, 0, 0, 0, 0],   // BK background
                                  [0, 0, 0, 1, 1, 2, 2, 2],   // EE excellent effort
                                  [0, 0, 0, 1, 1, 2, 3, 3],   // CA critical applications
                                  [0, 1, 1, 2, 2, 3, 4, 4],   // VI video
                                  [0, 1, 1, 2, 2, 3, 4, 5],   // VO voice
                                  [0, 1, 2, 3, 3, 4, 5, 6],   // IC internetwork control
                                  [0, 1, 2, 3, 4, 5, 6, 7]]); // NC network control
        int defaultGateIndex = default(0); // default gate index if no PCP found
        @class(PcpTrafficClassClassifier);
}
