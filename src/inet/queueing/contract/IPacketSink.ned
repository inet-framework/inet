//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet sink modules in the queueing framework.
// Packet sinks are terminal points for packets in a network that
// can operate in both passive and active modes simultaneously,
// providing maximum flexibility for packet consumption.
//
// Key features:
// - Serves as a terminal point for packet flows
// - Supports both push and pull packet processing modes
// - Can actively request packets from sources (pull mode)
// - Can passively receive packets from sources (push mode)
// - Provides a unified interface for packet consumption
// - Adapts to different packet processing paradigms
//
// @see ~IPassivePacketSink, ~IActivePacketSink, ~PacketSinkBase
//
moduleinterface IPacketSink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in @labels(push,pull);
}
