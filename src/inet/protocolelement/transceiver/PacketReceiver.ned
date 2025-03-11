//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingReceiverBase;
import inet.protocolelement.transceiver.contract.IPacketReceiver;

//
// Receives signals from the physical medium as complete units and forwards
// them to upper layers as whole packets. Implements a straightforward
// packet-based reception model.
//
// Operation:
// - Processes incoming signals as atomic units
// - Converts complete signals to packets
// - Handles reception as a single operation
// - Maintains packet boundaries during reception
// - Provides simple packet-based reception without streaming
//
// Particularly useful for:
// - Implementing basic packet-based receivers
// - Creating simple protocol stacks without streaming
// - Simulating traditional packet-based communication
// - Building receivers for protocols with fixed-size packets
// - Testing network behavior with atomic packet handling
//
// @see ~PacketTransmitter, ~DestreamingReceiver, ~StreamThroughReceiver
//
simple PacketReceiver extends StreamingReceiverBase like IPacketReceiver
{
    parameters:
        @class(PacketReceiver);
}
