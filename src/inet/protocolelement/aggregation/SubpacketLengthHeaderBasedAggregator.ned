//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation;

import inet.protocolelement.aggregation.base.AggregatorBase;
import inet.protocolelement.aggregation.contract.IPacketAggregator;

//
// Aggregates multiple packets into a single packet by inserting a length field
// header before each subpacket. This module collects packets according to the
// aggregation policy and combines them into a single aggregated packet. Each
// subpacket is preceded by a SubpacketLengthHeader that contains the length of
// the following subpacket, allowing the receiver to extract individual packets.
//
module SubpacketLengthHeaderBasedAggregator extends AggregatorBase like IPacketAggregator
{
    parameters:
        @class(SubpacketLengthHeaderBasedAggregator);
}
