//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.protectionmechanism;

import inet.common.SimpleModule;

//
// Implements the single protection mechanism for IEEE 802.11 networks as defined
// in the standard section 8.2.5.2. This module calculates the Duration/ID field
// values for different frame types (RTS, CTS, BlockAckReq, BlockAck, data,
// management) to properly set the Network Allocation Vector (NAV) in receiving
// stations.
//
simple SingleProtectionMechanism extends SimpleModule
{
    parameters:
        @class(SingleProtectionMechanism);
        string rateSelectionModule;
        @display("i=block/encrypt");
}

