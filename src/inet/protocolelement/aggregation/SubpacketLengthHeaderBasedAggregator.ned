//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation;

import inet.protocolelement.aggregation.base.AggregatorBase;
import inet.protocolelement.aggregation.contract.IPacketAggregator;

module SubpacketLengthHeaderBasedAggregator extends AggregatorBase like IPacketAggregator
{
    parameters:
        @class(SubpacketLengthHeaderBasedAggregator);
}
