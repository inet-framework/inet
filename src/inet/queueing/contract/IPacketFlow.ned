//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet flow modules in the queueing framework.
// Packet flows process packets passing through them without dropping
// any packets, but potentially modifying packet contents or metadata.
//
// Key features:
// - Acts as both a packet sink and a packet source
// - Supports both push and pull packet processing modes
// - Preserves packet count (no packet drops)
// - May modify packet contents or attached metadata
// - Provides a one-to-one connection between input and output
// - Maintains packet flow integrity while allowing transformations
//
// @see ~PacketLabeler, ~PacketTagger, ~PacketDelayer, ~OmittedPacketFlow
//
moduleinterface IPacketFlow extends IPacketSink, IPacketSource
{
    parameters:
        @omittedTypename(OmittedPacketFlow);
        @display("i=block/star");
}
