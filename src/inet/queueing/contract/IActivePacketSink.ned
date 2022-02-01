//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface must be implemented by active packet sink modules.
// An active packet sink, also called a collector, has one input that must be
// connected to a passive packet source module. The active packet sink is
// expected to pull packets from the connected passive packet source over time.
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
