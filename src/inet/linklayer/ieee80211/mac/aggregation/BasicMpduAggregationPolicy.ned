//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.aggregation;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IMpduAggregationPolicy;

//
// Selects a bounded ordered group for one immediate Block Ack exchange.
//
simple BasicMpduAggregationPolicy extends SimpleModule like IMpduAggregationPolicy
{
    parameters:
        @class(BasicMpduAggregationPolicy);
        int maxMpduCount = default(64); // Further bounded by the negotiated window and transport limits.
        @display("i=block/control");
}

