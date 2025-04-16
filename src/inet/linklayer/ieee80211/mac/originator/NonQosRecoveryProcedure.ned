//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;

//
// Implements frame retransmission and recovery procedures for non-QoS frames in
// IEEE 802.11 networks. Manages short and long retry counters, adjusts contention
// window size based on transmission outcomes, and enforces retry limits. Handles
// different retry procedures for frames below and above the RTS threshold as
// specified in IEEE 802.11 standard section 9.19.2.6.
//
// @see IRtsPolicy, Dcaf
//
simple NonQosRecoveryProcedure extends SimpleModule
{
    parameters:
        @class(NonQosRecoveryProcedure);
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

