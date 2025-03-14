//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketDuplicatorBase;


//
// Duplicates packets based on their ordinal number in the received packet stream.
// Provides precise control over which specific packets are duplicated, allowing
// for deterministic packet duplication patterns in simulations.
//
// Key features:
// - Selectively duplicates packets based on their position in the stream
// - Configurable via a vector of ordinal numbers
// - Deterministic behavior for reproducible simulations
// - Preserves all packet attributes and data in duplicates
// - Passes non-duplicated packets through unchanged
//
// Operation mechanism:
// - Maintains an internal counter tracking the ordinal number of each packet
// - When a packet's ordinal number matches an entry in the duplicatesVector,
//   the packet is duplicated (a copy is created and sent)
// - Both the original packet and its duplicate(s) are forwarded to the output
// - Packets with ordinal numbers not in the duplicatesVector pass through unchanged
//
// The duplicatesVector parameter specifies which packets to duplicate using
// a semicolon-separated list of ordinal numbers. For example, "5;6;12;13;"
// will duplicate the 5th, 6th, 12th, and 13th packets in the stream.
//
// @see ~PacketDuplicator, ~OrdinalBasedDropper, ~ContentBasedDuplicator
//
simple OrdinalBasedDuplicator extends PacketDuplicatorBase
{
    parameters:
        string duplicatesVector = default(""); // The duplicatesVector format is "<packetNumber>;<packetNumber>;...;"
                                               // Packet numbers in ascending order, e.g. "5;6;12;13;"
        @display("i=block/control");
        @class(OrdinalBasedDuplicator);
    gates:
        input in;
        output out;
}
