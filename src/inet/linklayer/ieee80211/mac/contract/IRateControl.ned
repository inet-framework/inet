//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 automatic rate control algorithms that dynamically 
// adjust transmission data rates based on channel conditions. Rate control modules 
// monitor transmission success/failure patterns and select appropriate data rates 
// to optimize throughput while maintaining reliability. These algorithms adapt to 
// changing wireless environments by increasing rates when conditions are favorable 
// and decreasing rates when errors occur.
//
// @see ~AarfRateControl, ~ArfRateControl, ~OnoeRateControl
//
moduleinterface IRateControl
{
    parameters:
        @display("i=block/cogwheel");
}
