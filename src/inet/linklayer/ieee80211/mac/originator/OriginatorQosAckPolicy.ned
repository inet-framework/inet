//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IOriginatorQosAckPolicy;

//
// Implements the default originator ACK policy for QoS stations.
//
simple OriginatorQosAckPolicy extends SimpleModule like IOriginatorQosAckPolicy
{
    parameters:
        @class(OriginatorQosAckPolicy);
        string rateSelectionModule;

        int blockAckReqThreshold = default(5);
        int maxBlockAckPolicyFrameLength @unit(B) = default(1000B);
        // Set only for an HT-or-later local STA when peer capability management establishes that the peer supports Compressed Block Ack.
        // This explicit assumption is needed because peer HT capabilities are not represented by the baseline agreement contract.
        bool assumePeerSupportsCompressedBlockAck = default(false);
        double blockAckTimeout @unit(s) = default(-1s);
        double ackTimeout @unit(s) = default(-1s);

        @display("i=block/control");
}
