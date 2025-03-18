//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.aggregation;

import inet.protocolelement.aggregation.base.DeaggregatorBase;
import inet.protocolelement.aggregation.contract.IPacketDeaggregator;

//
// Extracts multiple packets from a single aggregated packet by using the length
// field headers inserted before each subpacket. This module processes aggregated
// packets created by SubpacketLengthHeaderBasedAggregator, reading the length
// field from each SubpacketLengthHeader to determine the size of the following
// subpacket, and then extracting and forwarding each subpacket individually.
//
simple SubpacketLengthHeaderBasedDeaggregator extends DeaggregatorBase like IPacketDeaggregator
{
    parameters:
        @class(SubpacketLengthHeaderBasedDeaggregator);
}
