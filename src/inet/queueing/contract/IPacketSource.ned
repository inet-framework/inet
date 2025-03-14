//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet source modules in the queueing framework.
// Packet sources are starting points for packets in a network that
// can operate in both active and passive modes simultaneously,
// providing maximum flexibility for packet generation.
//
// Key features:
// - Serves as a starting point for packet flows
// - Supports both push and pull packet processing modes
// - Can actively push packets to sinks (push mode)
// - Can passively provide packets when requested (pull mode)
// - Provides a unified interface for packet generation
// - Adapts to different packet processing paradigms
//
// @see ~IPassivePacketSource, ~IActivePacketSource, ~PacketSourceBase
//
moduleinterface IPacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(push,pull);
}
