//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contention;

import inet.linklayer.ieee80211.mac.contract.IContention;

//
// The default implementation of IContention.
//
simple Contention like IContention
{
    parameters:
        bool initialChannelBusy = default(true); // assume that channel was busy before the simulation started
        bool backoffOptimization = default(true);    // we can pretend the frame has arrived into the queue a little bit earlier, and may be able to start transmitting immediately
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

