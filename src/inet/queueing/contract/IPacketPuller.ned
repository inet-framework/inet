//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet puller modules in the queueing framework.
// Packet pullers are key components in pull-based packet processing,
// where packet flow is driven by consumers requesting packets from
// producers.
//
// Key features:
// - Acts as both an active packet sink and a passive packet source
// - Operates in pull mode (active consumer, passive provider)
// - Initiates packet flow by pulling packets from upstream modules
// - Provides packets to downstream modules when requested
// - Serves as an adapter between pull-based and push-based processing
// - Maintains packet flow integrity during transfer
//
// @see ~PacketPullerBase, ~IPacketPusher, ~IActivePacketSink, ~IPassivePacketSource
//
moduleinterface IPacketPuller extends IActivePacketSink, IPassivePacketSource
{
    parameters:
        @display("i=block/star");
}
