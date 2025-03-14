//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.ratecontrol;

//
// Implements the Auto Rate Fallback (ARF) rate control algorithm for IEEE 802.11 
// networks. ARF is one of the earliest and most widely implemented rate adaptation 
// mechanisms. It increases the transmission rate after a certain number of consecutive 
// successful transmissions and decreases it after transmission failures. This simple 
// threshold-based approach allows stations to adapt to changing channel conditions 
// by finding an appropriate balance between throughput and reliability.
//
// @see ~IRateControl, ~AarfRateControl
//
simple ArfRateControl extends AarfRateControl
{
    parameters:
        increaseThresholdFactor = default(1);
        @display("i=block/cogwheel");
}
