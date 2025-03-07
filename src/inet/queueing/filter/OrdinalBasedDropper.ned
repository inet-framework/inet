//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;

//
// Deterministic packet dropper that selectively discards packets based on their
// position in the packet stream. This module enables precise control over which
// specific packets are dropped during a simulation.
//
// Operation mechanism:
// - Maintains a counter tracking the ordinal number of each packet in the stream
// - Compares each packet's ordinal number against the configured drop list
// - Drops packets whose ordinal numbers match entries in the dropsVector
// - Forwards all other packets to the connected consumer
//
// The dropsVector parameter specifies the ordinal numbers of packets to be dropped
// as a semicolon-separated list of ascending packet numbers (e.g., "5;6;12;13;").
//
// @see ~PacketFilter, ~ContentBasedFilter, ~RedDropper
//
simple OrdinalBasedDropper extends PacketFilterBase
{
    parameters:
        string dropsVector = default(""); // The dropsVector format is "<packetNumber>;<packetNumber>;...;"
                                          // Packet numbers in ascending order e.g. "5;6;12;13;"
        @class(OrdinalBasedDropper);
}
