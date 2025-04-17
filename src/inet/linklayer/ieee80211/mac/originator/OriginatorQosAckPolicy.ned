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
        double blockAckTimeout @unit(s) = default(-1s);
        double ackTimeout @unit(s) = default(-1s);

        @display("i=block/control");
}

