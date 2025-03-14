//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.aggregation;

import inet.linklayer.ieee80211.mac.contract.IMpduAggregationPolicy;

//
// Implements a basic IEEE 802.11n/ac MPDU aggregation policy that deliberately 
// disables frame aggregation. This implementation serves as a baseline configuration 
// that always returns individual MPDUs without combining them into A-MPDU frames. 
// It can be used in scenarios where MPDU aggregation is not desired or when testing 
// the performance impact of aggregation by comparing with aggregation-enabled policies.
//
// @see ~IMpduAggregationPolicy, ~BasicMsduAggregationPolicy
//
simple BasicMpduAggregationPolicy like IMpduAggregationPolicy
{
    @display("i=block/control");
}
