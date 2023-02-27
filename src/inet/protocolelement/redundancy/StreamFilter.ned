//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module implements a packet filter using solely the stream name. The stream
// name must match the filter criteria in order the packet to pass through the filter.
//
simple StreamFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        string streamNameFilter = default("*"); // filter expression for the stream name
        @class(StreamFilter);
}
