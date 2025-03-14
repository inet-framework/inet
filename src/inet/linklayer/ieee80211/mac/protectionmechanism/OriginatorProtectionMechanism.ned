//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.protectionmechanism;

//
// Implements the Duration/ID field calculation for IEEE 802.11 frames transmitted by non-QoS stations. 
// This module computes the appropriate duration values for RTS, data, and management frames according 
// to the IEEE 802.11 standard rules. For RTS frames, it calculates the time required to transmit the 
// pending frame plus control frames and inter-frame spaces. For data frames, it handles both fragmented 
// and non-fragmented cases, unicast and multicast transmissions. The module works with the rate selection 
// mechanism to determine accurate transmission times for frames at different data rates.
//
// @see ~SingleProtectionMechanism
//
simple OriginatorProtectionMechanism
{
    parameters:
        string rateSelectionModule;
        @display("i=block/encrypt");
}
