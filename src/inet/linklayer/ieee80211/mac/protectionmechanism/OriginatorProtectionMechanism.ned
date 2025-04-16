//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.protectionmechanism;

import inet.common.SimpleModule;

//
// Calculates the Duration/ID field for IEEE 802.11 frames to implement the
// Network Allocation Vector (NAV) mechanism for virtual carrier sensing. This
// module computes appropriate duration values for different frame types (RTS,
// data, management) based on IEEE 802.11 standard rules. For RTS frames, it
// calculates time needed for the complete RTS-CTS-DATA-ACK exchange.
//
simple OriginatorProtectionMechanism extends SimpleModule
{
    parameters:
        @class(OriginatorProtectionMechanism);
        string rateSelectionModule;
        @display("i=block/encrypt");
}

