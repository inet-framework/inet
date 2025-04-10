//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.aggregation;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IMpduAggregationPolicy;

//
// Implements an MPDU aggregation policy that never aggregates frames.
//
simple BasicMpduAggregationPolicy extends SimpleModule like IMpduAggregationPolicy
{
    parameters:
        @class(BasicMpduAggregationPolicy);
        @display("i=block/control");
}

