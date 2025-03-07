//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for passive packet source modules in the queueing framework.
// Passive packet sources are starting points for packets in pull-based
// packet processing, where packet flow is driven by consumers requesting
// packets from producers.
//
// Key features:
// - Operates in pull mode (passive provider)
// - Provides packets when requested by active packet sinks
// - Serves as a starting point for packet flows
// - Responds to packet requests rather than initiating transfers
// - May generate or retrieve packets according to various patterns
// - Can implement various packet generation strategies
//
// @see ~PassivePacketSource, ~PacketQueue, ~PacketScheduler, ~IActivePacketSink
//
moduleinterface IPassivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(pull);
}
