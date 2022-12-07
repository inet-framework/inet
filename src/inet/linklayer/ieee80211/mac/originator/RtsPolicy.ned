//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IRtsPolicy;


//
// Implements the default RTS policy.
//
simple RtsPolicy like IRtsPolicy
{
    parameters:
        string rateSelectionModule;
        double ctsTimeout @unit(s) = default(-1s);
        int rtsThreshold @unit(B) = default(2346B);
        @display("i=block/control");
}

