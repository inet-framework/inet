//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.policy;

import inet.protocolelement.aggregation.contract.IAggregatorPolicy;

simple LengthBasedAggregatorPolicy like IAggregatorPolicy
{
    parameters:
        int minNumSubpackets = default(0);
        int maxNumSubpackets;
        int minAggregatedLength @unit(b) = default(0b);
        int maxAggregatedLength @unit(b);
        @display("i=block/star");
        @class(LengthBasedAggregatorPolicy);
}
