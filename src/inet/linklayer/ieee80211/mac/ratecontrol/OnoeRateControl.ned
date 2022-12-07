//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.ratecontrol;

import inet.linklayer.ieee80211.mac.contract.IRateControl;

//
// Implements ONOE, a credit-based rate control algorithm originally
// developed by Atheros.
//
simple OnoeRateControl like IRateControl
{
    parameters:
        double initialRate @unit(bps) = default(-1bps); // -1 means fastest mandatory rate
        double interval @unit(s) = default(1s);
        @display("i=block/cogwheel");
        @signal[datarateChanged];
        @statistic[datarateChanged](title="datarate"; record=vector; interpolationmode=sample-hold);
}

