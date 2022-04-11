//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.base;

import inet.protocolelement.aggregation.contract.IAggregatorPolicy;
import inet.queueing.base.PacketPusherBase;

module AggregatorBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        string aggregatorPolicyClass = default("");
        string aggregatorPolicyModule = default(".aggregatorPolicy");
        @display("i=block/join");
        @class(AggregatorBase);
    submodules:
        aggregatorPolicy: <default("LengthBasedAggregatorPolicy")> like IAggregatorPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
}
