//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

//
// Implements the IEEE 802.11 frame recovery procedure for non-QoS stations. This module 
// manages retransmission attempts when frames are not acknowledged, applying exponential 
// backoff to the contention window size after each failed transmission. It enforces 
// short and long retry limits (7 and 4 by default) for different frame types - short 
// retry limit for frames shorter than RTSThreshold and long retry limit for longer frames. 
// The module tracks statistics on contention window changes and retry limit events, 
// and signals when a frame has reached its retry limit and must be discarded.
//
// @see ~QosRecoveryProcedure, ~AckHandler
//
simple NonQosRecoveryProcedure
{
    parameters:
        string cwCalculatorModule;
        string rtsPolicyModule;
        int shortRetryLimit = default(7);
        int longRetryLimit = default(4);
        @display("i=block/cogwheel");
        @signal[contentionWindowChanged](type=long);
        @signal[retryLimitReached](type=inet::Packet);
        @statistic[contentionWindowChanged](title="contention window"; record=vector; interpolationmode=sample-hold);
        @statistic[retryLimitReached](title="retry limit reached"; record=count);
}
