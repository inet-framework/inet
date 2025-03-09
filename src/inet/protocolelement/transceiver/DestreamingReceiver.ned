//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.PacketReceiverBase;
import inet.protocolelement.transceiver.contract.IPacketReceiver;

//
// Receives signals from the physical medium as streams but delivers them to
// upper layers as whole packets. Converts streaming input to packet-based output.
//
// Operation:
// - Processes incoming signal streams incrementally
// - Receives signal start and end events separately
// - Buffers signal parts until complete
// - Assembles complete packets from signal streams
// - Delivers whole packets to upper layers
//
// Particularly useful for:
// - Implementing receivers that interface between streaming and packet domains
// - Creating receivers for preemptible transmission systems
// - Supporting time-sensitive networking with packet-based upper layers
// - Building protocol stacks with mixed streaming/packet processing
// - Converting stream-based physical layer to packet-based data link layer
//
// @see ~StreamingTransmitter, ~PacketReceiver, ~StreamThroughReceiver
//
simple DestreamingReceiver extends PacketReceiverBase like IPacketReceiver
{
    parameters:
        @class(DestreamingReceiver);
}
