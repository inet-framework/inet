//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for passive packet source modules.
// A passive packet source, also called a provider, has one output that must be
// connected to an active packet sink module. The connected active packet sink
// is expected to pull packets from the passive packet source over time.
//
// In the pull handshake, the collector decides when a packet moves, and the
// provider holds the packets and chooses which packet moves. The acceptance
// condition still belongs to the sink: the collector can give the provider a
// filter, and the provider then chooses as if it held only the packets that
// the filter matches. So in both handshakes, the search happens where the
// packets are. The provider notifies the collector when its answer can change.
// The push handshake of ~IPassivePacketSink is the mirror image.
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
