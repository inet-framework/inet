//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contention;

import inet.linklayer.ieee80211.mac.contract.IContention;

//
// Default implementation of the IEEE 802.11 contention mechanism that provides 
// CSMA/CA functionality. Manages the timing of channel access by implementing 
// interframe spaces (DIFS/AIFS/EIFS), random backoff procedures, and channel 
// sensing. Tracks contention states (IDLE, DEFER, IFS_AND_BACKOFF) and provides 
// notifications when channel access is granted. Essential component for both DCF 
// and EDCA channel access methods.
//
// @see ~IContention, ~Dcaf, ~Edcaf
//
simple Contention like IContention
{
    parameters:
        bool initialChannelBusy = default(true); // Assume that the channel was busy before the simulation started
        bool backoffOptimization = default(true);    // We can pretend the frame has arrived into the queue a little bit earlier, and maybe able to start transmitting immediately
        @display("i=block/timer");
        @signal[backoffPeriodGenerated](type=long);
        @signal[backoffStarted](type=simtime_t);
        @signal[backoffStopped](type=simtime_t);
        @signal[channelAccessGranted];
        @signal[stateChanged](type=long; enum=inet::ieee80211::Contention::State);
        @statistic[backoffPeriodGenerated](title="backoff periods"; record=vector; interpolationmode=none);
        @statistic[backoffActive](title="backoff active"; source=warmup(count(backoffStarted)-count(backoffStopped)); record=vector; interpolationmode=sample-hold; autoWarmupFilter=false);
        @statistic[channelAccessGranted](title="channel access granted"; record=count,vector?);
        @statistic[stateChanged](title="contention state changed"; type=enum; enum=IDLE, DEFER, IFS_AND_BACKOFF; record=count,vector; interpolationmode=none);
        @defaultStatistic(stateChanged:vector);
}
