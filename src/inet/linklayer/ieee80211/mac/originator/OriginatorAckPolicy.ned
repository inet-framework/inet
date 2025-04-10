//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IOriginatorAckPolicy;

//
// Implements the default originator ACK policy for non-QoS stations.
//
simple OriginatorAckPolicy extends SimpleModule like IOriginatorAckPolicy
{
    parameters:
        @class(OriginatorAckPolicy);
        string rateSelectionModule;
        double ackTimeout @unit(s) = default(-1s);
        @display("i=block/control");
}

