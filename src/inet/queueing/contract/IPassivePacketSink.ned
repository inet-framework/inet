//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// This module interface must be implemented by passive packet sink modules.
// A passive packet sink, also called a consumer, has one input that must be
// connected to an active packet source module. The connected active packet
// source is expected to push packets into the passive packet sink over time.
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
