//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.ratecontrol;

import inet.linklayer.ieee80211.mac.contract.IRateControl;

//
// Implements the ONOE rate control algorithm for IEEE 802.11 networks, a credit-based 
// approach originally developed by Atheros. ONOE evaluates transmission performance 
// over fixed time intervals rather than on a per-packet basis. It uses a credit system 
// to make rate adjustment decisions, increasing rates when error rates are low and 
// decreasing them when errors exceed a threshold. This more conservative approach 
// provides stable rate adaptation with fewer oscillations than threshold-based 
// algorithms like ARF.
//
// @see ~IRateControl, ~AarfRateControl, ~ArfRateControl
//
simple OnoeRateControl like IRateControl
{
    parameters:
        double initialRate @unit(bps) = default(-1bps); // -1 means the fastest mandatory rate
        double interval @unit(s) = default(1s);
        @display("i=block/cogwheel");
        @signal[datarateChanged];
        @statistic[datarateChanged](title="datarate"; record=vector; interpolationmode=sample-hold);
}
