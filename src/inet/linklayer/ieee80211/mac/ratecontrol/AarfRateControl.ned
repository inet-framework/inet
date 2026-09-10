//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.ratecontrol;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRateControl;

//
// Implements the Adaptive ARF (AARF) rate control mechanism, which was
// initially described in IEEE 802.11 Rate Adaptation: A Practical Approach,
// by M. Lacage, M.H. Manshaei, and T. Turletti, 2004.
// Follows INRIA RR-5208 Appendix A with per-receiver state. Counters saturate
// and threshold checks use >=; ordinary fallback extends to every configured
// retry multiple, beyond the appendix's explicit list ending at ten.
//
simple AarfRateControl extends SimpleModule like IRateControl
{
    parameters:
        @class(AarfRateControl);
        double initialRate @unit(bps) = default(-1bps); // -1 means the fastest mandatory rate
        int minTimerThreshold = default(15); // Minimum packet timer threshold (>0); checked on success before counting that attempt. Idle time and rate queries do not advance it.
        double timerThresholdFactor = default(1.5); // On a failed probe, timer threshold becomes max(minTimerThreshold, floor(factor * updated increaseThreshold)); finite and >=1.
        int increaseThreshold = default(10); // Consecutive successful attempts needed to increase the rate (>0)
        int decreaseThreshold = default(2); // Outside recovery, decrease on each multiple of this per-packet retry count (>0)
        double increaseThresholdFactor = default(2); // When the transmission of the probing packet fails, increaseThreshold is multiplied by increaseThresholdFactor.
        int maxIncreaseThreshold = default(50); // Upper bound for increaseThreshold.
        displayStringTextFormat = default("{numStations} stations");
        @display("i=block/cogwheel");
        @signal[datarateChanged];
        @statistic[datarateChanged](title="datarate"; record=vector; interpolationmode=sample-hold); // aggregate rate over all stations (interleaved)
        @statistic[dataratePerStation](title="data rate per station"; source=demux(datarateChanged); unit=bps; record=vector; interpolationmode=sample-hold); // one vector per receiver, demultiplexed by station
}

