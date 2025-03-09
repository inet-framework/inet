//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.contract;

//
// Defines the interface for aggregator policy modules that determine when and
// how packets should be aggregated into larger units for transmission.
//
// Operation:
// - Evaluates incoming packets for aggregation eligibility
// - Determines when to trigger packet aggregation
// - Decides which packets should be combined together
// - Provides aggregation parameters and constraints
// - Supports different aggregation strategies through implementations
//
// @see ~AggregatorBase, ~LengthBasedAggregatorPolicy
//
moduleinterface IAggregatorPolicy
{
    parameters:
        @display("i=block/star");
}
