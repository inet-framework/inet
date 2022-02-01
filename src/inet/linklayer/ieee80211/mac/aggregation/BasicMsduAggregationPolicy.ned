//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.aggregation;

import inet.linklayer.ieee80211.mac.contract.IMsduAggregationPolicy;

simple BasicMsduAggregationPolicy like IMsduAggregationPolicy
{
    int subframeNumThreshold = default(-1); // Minimum number of subframes needed to compose an A-MSDU. (-1 means it won't be checked)
    int aggregationLengthThreshold = default(-1); // The minimum length for an aggregated payload necessary to create an A-MSDU (-1 means it won't be checked).
    int maxAMsduSize = default(4065); // -1 means infinity
    bool qOsCheck = default(true); // Turns on/off the check of the presence of the QoS flag (false means off -- this is needed for aggregation in DCF mode).
    @display("i=block/control");
}

