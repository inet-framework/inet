//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Packet classifier that routes packets based on IEEE 802.1Q Priority Code Point (PCP) values. 
// Examines PCP tags (values 0-7) attached to packets and forwards them to different output gates 
// according to a configurable mapping. Supports different operating modes to consider request tags, 
// indication tags, or both, enabling flexible quality of service implementations in Ethernet networks.
//
// @see ~IPacketClassifier, ~PcpTrafficClassClassifier, ~PcpTag
//
simple PcpClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        object pcpToGateIndex; // Specifies the gate index for all PCP values in ascending order
        int defaultGateIndex = default(0); // Default gate index if no PCP is found
        @class(PcpClassifier);
}
