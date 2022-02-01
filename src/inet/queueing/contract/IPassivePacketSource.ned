//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface must be implemented by passive packet source modules.
// A passive packet source, also called a provider, has one output that must be
// connected to an active packet sink module. The connected active packet sink
// is expected to pull packets from the passive packet source over time.
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
