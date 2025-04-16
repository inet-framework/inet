//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

//
// Implements frame retransmission and recovery procedures for QoS frames in IEEE
// 802.11 networks. Manages retry counters for failed transmissions, adjusts
// contention window size, and enforces retry limits. Maintains separate short and
// long retry counters based on frame size relative to RTS threshold, and tracks
// per-TID frame status to support Quality of Service requirements.
//
// @see IRtsPolicy, Dcaf, Edcaf
//
simple QosRecoveryProcedure
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

