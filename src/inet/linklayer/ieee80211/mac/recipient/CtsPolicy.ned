//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.linklayer.ieee80211.mac.contract.ICtsPolicy;


//
// Implements the default CTS policy.
//
simple CtsPolicy like ICtsPolicy
{
    parameters:
        string rxModule;
        string rateSelectionModule;
        @display("i=block/control");
}

