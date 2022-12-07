//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.aggregation;

import inet.linklayer.ieee80211.mac.contract.IMpduAggregationPolicy;

//
// Implements an MPDU aggregation policy which never aggregates frames.
//
simple BasicMpduAggregationPolicy like IMpduAggregationPolicy
{
    @display("i=block/control");
}

