//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketDuplicatorBase;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPassivePacketSink;

//
// Creates exact copies of packets and forwards both the original and duplicates
// to the same output. Provides a simple way to simulate packet duplication that
// occurs in real networks due to various transmission issues.
//
// Key features:
// - Configurable number of duplicates per packet
// - Preserves all packet attributes and data in duplicates
// - Operates in push mode (passive sink, active source)
// - Supports both constant and random (volatile) duplication counts
// - Maintains packet order with duplicates following the original
//
// Operation mechanism:
// - When a packet arrives at the input, the module creates the specified
//   number of exact copies
// - Both the original packet and all duplicates are pushed to the output
//   in sequence
// - Each copy is identical to the original packet, including all headers,
//   data, and metadata
// - Setting numDuplicates to 0 means no duplication occurs (pass-through mode)
//
// Note: Unlike PacketCloner which sends copies to different outputs,
// PacketDuplicator sends all copies to the same output, effectively
// implementing packet duplication rather than distribution.
//
// @see ~OrdinalBasedDuplicator, ~PacketCloner, ~ContentBasedDuplicator
//
simple PacketDuplicator extends PacketDuplicatorBase like IPassivePacketSink, IActivePacketSource
{
    parameters:
        volatile int numDuplicates = default(1); // Number of duplicates, 0 means only the original packet is forwarded
        @class(PacketDuplicator);
        @display("i=block/star");
    gates:
        input in @labels(push);
        output out @labels(push);
}
