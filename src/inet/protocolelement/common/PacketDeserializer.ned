//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;

//
// Takes a serialized packet and deserializes it by using the protocol dissector
// registry to break down the packet into its component chunks based on the
// protocol. It finds the protocol tag in the packet, creates a chunk builder to
// reconstruct the packet, uses a packet dissector to break down the packet
// according to the protocol, erases all existing content from the packet, and
// inserts the reconstructed content at the front of the packet.
//
simple PacketDeserializer extends PacketFlowBase
{
    parameters:
        @display("i=block/cogwheel");
        @class(PacketDeserializer);
}
