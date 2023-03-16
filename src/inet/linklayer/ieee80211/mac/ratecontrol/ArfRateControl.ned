//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.ratecontrol;

//
// Implements the Auto Rate Fallback (ARF) adaptive rate
// control mechanism.
//
simple ArfRateControl extends AarfRateControl
{
    parameters:
        increaseThresholdFactor = default(1);
        @display("i=block/cogwheel");
}

