//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet source modules. Packets are created
// randomly with regard to packet length and packet data. The representation of
// packet data can also be configured.
//
// Available packet name format directives:
// source, application, network node
// display name or full name, full path
//
//  - %a automatic application display name or application full name or module display name or module full name
//  - %n module display name or module full name
//  - %m application display name or application full name
//  - %M network node display name or network node full name
//  - %p module full path
//  - %h application full path
//  - %H network node full path
//  - %c packet number
//  - %l packet length
//  - %d packet data
//  - %t simulation time
//  - %e event number
//
// @see ~IPacketSource
//
simple PacketSourceBase extends PacketProcessorBase
{
    parameters:
        string packetNameFormat = default("%a-%c"); // see directives in module documentation
        string packetRepresentation @units("bitCount","bits","byteCount","bytes","applicationPacket") = default("byteCount"); // determines the chunk of the packet data
        string packetProtocol = default("unknown");
        volatile int packetLength @unit(b);
        volatile int packetData = default(-1);
        bool attachCreationTimeTag = default(true);
        bool attachIdentityTag = default(true);
        bool attachDirectionTag = default(true);
        displayStringTextFormat = default("created %p pk (%l)");
        @class(PacketSourceBase);
        @display("i=block/source");
        @signal[packetCreated](type=inet::Packet);
}
