//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.base;

import inet.protocolelement.aggregation.contract.IAggregatorPolicy;
import inet.queueing.base.PacketPusherBase;

//
// Base module for packet aggregation. Collects multiple packets and combines them
// into a single aggregated packet based on a configurable aggregation policy.
// Aggregation improves network efficiency by reducing header overhead when
// transmitting multiple small packets. The module delegates the aggregation
// decision to a policy class or module that determines if a packet can be
// aggregated with the current set.
//
// @see DeaggregatorBase
//
module AggregatorBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        string aggregatorPolicyClass = default("");
        string aggregatorPolicyModule = default(".aggregatorPolicy");  // Only used if aggregatorPolicyClass==""
        @display("i=block/join");
        @class(AggregatorBase);
    submodules:
        aggregatorPolicy: <default("LengthBasedAggregatorPolicy")> like IAggregatorPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
}
