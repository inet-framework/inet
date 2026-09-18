//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.ICtsPolicy;

//
// Implements the default CTS policy.
//
simple CtsPolicy extends SimpleModule like ICtsPolicy
{
    parameters:
        string modeSetModule = default(""); // Required catalog provider; supplied by the enclosing MAC // Configured MAC catalog provider
        @class(CtsPolicy);
        string rxModule;
        string rateSelectionModule;
        @display("i=block/control");
}

