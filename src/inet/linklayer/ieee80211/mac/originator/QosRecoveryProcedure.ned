//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

//
// Implements the IEEE 802.11e/n/ac QoS frame recovery procedure for QoS stations. 
// This module extends the basic recovery procedure with QoS-specific features, 
// managing retransmission attempts on a per-TID (Traffic ID) basis. It applies 
// differentiated retry limits and backoff parameters based on traffic priority 
// levels, supporting the enhanced distributed channel access (EDCA) mechanism. 
// The module works with the QosAckHandler to track acknowledgment status for 
// different access categories and maintains separate contention windows for each 
// traffic class, ensuring appropriate QoS guarantees for different traffic types.
//
// @see ~NonQosRecoveryProcedure, ~QosAckHandler
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
