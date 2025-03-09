//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;

//
// Converts serialized packet data back into structured protocol-specific chunks
// by using protocol dissectors to interpret the raw packet content.
//
// Operation:
// - Identifies the packet's protocol using the PacketProtocolTag
// - Uses the protocol dissector registry to parse the packet content
// - Reconstructs the packet with properly structured protocol chunks
// - Maintains the original packet data while providing protocol-aware representation
//
// Particularly useful for:
// - Processing packets received from physical layer simulations
// - Analyzing packet contents after network transmission
// - Enabling protocol-specific processing after packet serialization
// - Implementing protocol analyzers and packet inspection tools
// - Converting between different packet representations in protocol stacks
//
// @see ~PacketSerializer, ~PacketFlowBase
//
simple PacketDeserializer extends PacketFlowBase
{
    parameters:
        @display("i=block/cogwheel");
        @class(PacketDeserializer);
}
