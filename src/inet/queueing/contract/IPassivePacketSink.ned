//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for passive packet sink modules.
// A passive packet sink, also called a consumer, has one input that must be
// connected to an active packet source module. The connected active packet
// source is expected to push packets into the passive packet sink over time.
//
// In the push handshake, the producer holds the packets and decides when a
// packet moves, and the consumer decides which packets it accepts. The
// producer can ask whether the consumer accepts some packet or a specific
// packet, and it can try another packet that it holds. The consumer notifies
// the producer when its answer can change. The pull handshake of
// ~IPassivePacketSource is the mirror image.
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
