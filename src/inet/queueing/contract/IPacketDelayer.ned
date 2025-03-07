//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet delayer modules in the queueing framework.
// Packet delayers introduce a configurable time delay between
// receiving a packet and forwarding it, simulating various delay
// sources in network communication.
//
// Key features:
// - Acts as both a passive packet sink and an active packet source
// - Introduces a configurable delay for each packet
// - Preserves packet content and attributes during the delay
// - May support fixed, variable, or distribution-based delays
// - Can model various types of network delays (transmission, propagation, etc.)
//
// @see ~PacketDelayer, ~OmittedPacketDelayer
//
moduleinterface IPacketDelayer extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @omittedTypename(OmittedPacketDelayer);
        @display("i=block/delay");
}
