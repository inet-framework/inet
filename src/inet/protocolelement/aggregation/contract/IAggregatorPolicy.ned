//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.contract;

//
// Interface for aggregator policy modules that determine when packets can be
// aggregated. Implementations of this interface decide whether a new packet can be
// combined with the current set of aggregated packets based on various criteria
// such as size limits, timing constraints, or protocol-specific requirements.
// The policy module is consulted by the aggregator to make aggregation decisions.
//
// @see SubpacketLengthHeaderBasedAggregator
//
moduleinterface IAggregatorPolicy
{
    parameters:
        @display("i=block/star");
}
