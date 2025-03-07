//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for passive packet sink modules in the queueing framework.
// Passive packet sinks are terminal points for packets in push-based
// packet processing, where packet flow is driven by producers sending
// packets to consumers.
//
// Key features:
// - Operates in push mode (passive consumer)
// - Receives packets from active packet sources
// - Serves as a terminal point for packet flows
// - Reacts to incoming packets rather than requesting them
// - May process, collect statistics, or simply consume packets
// - Can implement various packet handling strategies
//
// @see ~PassivePacketSink, ~PacketQueue, ~PacketClassifier, ~IActivePacketSource
//
moduleinterface IPassivePacketSink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in @labels(push);
}
