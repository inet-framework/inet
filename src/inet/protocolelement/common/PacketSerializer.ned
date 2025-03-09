//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;

//
// Converts structured protocol-specific packet chunks into a flat byte
// representation, serializing the packet content into a raw binary format.
//
// Operation:
// - Takes packets with potentially multiple protocol-specific chunks
// - Converts all packet data into a single BytesChunk representation
// - Preserves the complete packet content in serialized form
// - Maintains packet metadata while flattening the data structure
//
// @see ~PacketDeserializer, ~PacketFlowBase
//
simple PacketSerializer extends PacketFlowBase
{
    parameters:
        @display("i=block/cogwheel");
        @class(PacketSerializer);
}
