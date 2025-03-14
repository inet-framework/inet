//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.aggregation;

import inet.linklayer.ieee80211.mac.contract.IMsduAggregationPolicy;

//
// Implements a configurable IEEE 802.11n/ac MSDU aggregation policy that combines 
// multiple MAC Service Data Units into a single A-MSDU frame based on several 
// parameters. The policy evaluates frame aggregation opportunities using thresholds 
// for minimum subframe count, minimum aggregated payload length, and maximum A-MSDU 
// size. It can be configured to check QoS compatibility between frames or to operate 
// in DCF mode by disabling QoS checks. This implementation provides flexible control 
// over the aggregation process to balance throughput improvements against delay 
// considerations.
//
// @see ~IMsduAggregationPolicy, ~BasicMpduAggregationPolicy
//
simple BasicMsduAggregationPolicy like IMsduAggregationPolicy
{
    int subframeNumThreshold = default(-1); // Minimum number of subframes needed to compose an A-MSDU. (-1 means it won't be checked)
    int aggregationLengthThreshold = default(-1); // The minimum length for an aggregated payload necessary to create an A-MSDU (-1 means it won't be checked).
    int maxAMsduSize = default(4065); // -1 means infinity
    bool qOsCheck = default(true); // Turns on/off the check of the presence of the QoS flag (false means off -- this is needed for aggregation in DCF mode).
    @display("i=block/control");
}
