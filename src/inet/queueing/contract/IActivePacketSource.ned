//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface must be implemented by active packet source modules.
// An active packet source, also called a producer, has one output that must
// be connected to a passive packet sink module. The active packet source is
// expected to push packets into the connected passive packet sink over time.
//
// @see ~PassivePacketSink, ~PacketServer, ~PacketClassifier, ~IPassivePacketSink
//
moduleinterface IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out @labels(push);
}
