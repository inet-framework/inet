//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IOriginatorAckPolicy;


//
// Implements the default originator ACK policy for non-QoS stations.
//
simple OriginatorAckPolicy like IOriginatorAckPolicy
{
    parameters:
        string rateSelectionModule;
        double ackTimeout @unit(s) = default(-1s);
        @display("i=block/control");
}

