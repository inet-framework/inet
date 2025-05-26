//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.channelaccess;

import inet.common.SimpleModule;

//
// Implements HCCA (Hybrid Coordination Function Controlled Channel Access)
// for IEEE 802.11.
//
// IMPORTANT: This module is currently a placeholder. INET currently does not
// implement HCCA.
//
simple Hcca extends SimpleModule
{
    parameters:
        @class(Hcca);
        @display("i=block/star");
}
