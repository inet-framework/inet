//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Advanced classifier that maps IEEE 802.1Q Priority Code Point (PCP) values to traffic classes 
// according to IEEE 802.1Q standard recommendations. Uses a two-dimensional mapping table where 
// rows represent PCP values (0-7) and columns represent the number of available traffic classes. 
// Dynamically adapts to the number of connected output gates, implementing the standard's recommended 
// priority-to-traffic class mappings.
//
// @see ~PcpClassifier, ~IPacketClassifier, ~PcpTag
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
        int defaultGateIndex = default(0); // Default gate index if no PCP found
        @class(PcpTrafficClassClassifier);
}
