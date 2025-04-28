//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.protectionmechanism;

import inet.common.SimpleModule;

simple OriginatorProtectionMechanism extends SimpleModule
{
    parameters:
        @class(OriginatorProtectionMechanism);
        string rateSelectionModule;
        @display("i=block/encrypt");
}

