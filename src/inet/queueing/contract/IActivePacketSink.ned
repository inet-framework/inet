//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for active packet sink modules in the queueing framework.
// Active packet sinks are terminal points for packets in pull-based
// packet processing, where packet flow is driven by consumers requesting
// packets from producers.
//
// Key features:
// - Operates in pull mode (active consumer)
// - Actively requests packets from passive packet sources
// - Controls the timing and rate of packet collection
// - Initiates packet flow in the network
// - Serves as a terminal point for packet flows
// - Determines when packets are processed in the system
//
// @see ~ActivePacketSink, ~PacketServer, ~PacketScheduler, ~IPassivePacketSource
//
moduleinterface IActivePacketSink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in @labels(pull);
}
