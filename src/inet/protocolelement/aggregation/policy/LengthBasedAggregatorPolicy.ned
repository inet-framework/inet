//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.policy;

import inet.common.SimpleModule;
import inet.protocolelement.aggregation.contract.IAggregatorPolicy;

//
// A policy for aggregating subpackets based on their length
//
simple LengthBasedAggregatorPolicy extends SimpleModule like IAggregatorPolicy
{
    parameters:
        @class(LengthBasedAggregatorPolicy);
        int minNumSubpackets = default(0);
        int maxNumSubpackets;
        int minAggregatedLength @unit(b) = default(0b);
        int maxAggregatedLength @unit(b);
        @display("i=block/star");
        @class(LengthBasedAggregatorPolicy);
}
