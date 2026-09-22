//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRtsPolicy;

//
// Implements the default RTS policy.
//
simple RtsPolicy extends SimpleModule like IRtsPolicy
{
    parameters:
        string modeSetModule = default(""); // Required catalog provider; supplied by the enclosing MAC // Configured MAC catalog provider
        @class(RtsPolicy);
        string rateSelectionModule;
        double ctsTimeout @unit(s) = default(-1s);
        int rtsThreshold @unit(B) = default(2346B);
        @display("i=block/control");
}

