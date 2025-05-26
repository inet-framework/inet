//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRtsPolicy;

//
// Implements the default RTS policy for QoS stations.
//
simple QosRtsPolicy extends SimpleModule like IRtsPolicy
{
    parameters:
        @class(QosRtsPolicy);
        string rateSelectionModule;
        double ctsTimeout @unit(s) = default(-1s);
        int rtsThreshold @unit(B) = default(2346B);
        @display("i=block/control");
}

