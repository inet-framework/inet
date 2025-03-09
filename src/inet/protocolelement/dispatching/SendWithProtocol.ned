//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.dispatching;

//
// Encapsulates outgoing packets with protocol headers containing protocol
// identification information. Acts as a protocol multiplexer in layered
// protocol stacks.
//
// Operation:
// - Extracts protocol information from the packet's PacketProtocolTag
// - Creates a ProtocolHeader with the appropriate protocol ID
// - Inserts the protocol header at the front of the packet
// - Registers with protocol services to handle protocol multiplexing
// - Forwards the encapsulated packet to lower layers
//
// @see ~ReceiveWithProtocol, ~ProtocolHeader
//
simple SendWithProtocol
{
    parameters:
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
