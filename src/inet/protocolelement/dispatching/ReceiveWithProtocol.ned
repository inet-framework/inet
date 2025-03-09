//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.dispatching;

//
// Extracts protocol information from incoming packets and attaches appropriate
// protocol tags for upper layer processing. Acts as a protocol demultiplexer
// in layered protocol stacks.
//
// Operation:
// - Removes protocol header from the front of incoming packets
// - Identifies the encapsulated protocol using the protocol ID in the header
// - Adds PacketProtocolTag to indicate the protocol of the packet content
// - Adds DispatchProtocolReq tag to guide further packet processing
// - Forwards the packet to the appropriate upper layer protocol handler
//
// @see ~SendWithProtocol, ~ProtocolHeader
//
simple ReceiveWithProtocol
{
    parameters:
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
