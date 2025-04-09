//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.base;

import inet.queueing.base.PacketPusherBase;

//
// Base module for packet deaggregation. Takes an aggregated packet and splits it
// back into its original subpackets. Deaggregation is the reverse process of
// aggregation, extracting individual packets that were previously combined to
// reduce header overhead. The module receives an aggregated packet, extracts the
// individual subpackets, and forwards each one separately through its output gate.
//
// @see AggregatorBase
//
simple DeaggregatorBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        @display("i=block/fork");
        @class(DeaggregatorBase);
}
