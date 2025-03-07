//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet pusher modules in the queueing framework.
// Packet pushers are key components in push-based packet processing,
// where packet flow is driven by producers sending packets to consumers.
//
// Key features:
// - Acts as both a passive packet sink and an active packet source
// - Operates in push mode (passive consumer, active provider)
// - Receives packets from upstream modules and forwards them downstream
// - Maintains packet flow integrity during transfer
// - Serves as an adapter between different packet processing stages
// - May implement various forwarding or processing logic
//
// @see ~PacketPusherBase, ~IPacketPuller, ~IPassivePacketSink, ~IActivePacketSource
//
moduleinterface IPacketPusher extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @display("i=block/star");
}
