//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet source components in the queueing framework.
// Packet sources generate packets with configurable content and attributes,
// serving as the starting points for packet flows in network simulations.
//
// This module provides extensive configuration options for:
// - Packet naming with customizable format directives
// - Packet length and data content generation
// - Packet representation and protocol specification
// - Automatic tag attachment for tracking and identification
//
// Available packet name format directives:
//
//  - %a: automatic application display name or application full name or module display name or module full name
//  - %n: module display name or module full name
//  - %m: application display name or application full name
//  - %M: network node display name or network node full name
//  - %p: module full path
//  - %h: application full path
//  - %H: network node full path
//  - %c: packet number
//  - %l: packet length
//  - %d: packet data
//  - %t: simulation time
//  - %e: event number
//
// Derived modules can implement specific packet generation strategies
// by defining when packets are created and how they are forwarded.
//
// @see ~IPacketSource, ~ActivePacketSource, ~PassivePacketSource
//
simple PacketSourceBase extends PacketProcessorBase
{
    parameters:
        // Packet naming and identification
        string packetNameFormat = default("%a-%c"); // Format string for packet names using directives listed in module documentation
        bool attachIdentityTag = default(true);     // When true, attaches an identifier tag to created packets

        // Packet content configuration
        string packetRepresentation @units("bitCount","bits","byteCount","bytes","applicationPacket") = default("byteCount"); // Determines the chunk type for packet data
        string packetProtocol = default("unknown"); // Protocol identifier for created packets
        volatile int packetLength @unit(b);         // Length of generated packets in bits
        volatile int packetData = default(-1);      // Data value to include in packets (-1 for random)

        // Tag configuration
        bool attachCreationTimeTag = default(true); // When true, attaches creation time information to packets
        bool attachDirectionTag = default(true);    // When true, attaches direction information to packets

        // Display configuration
        displayStringTextFormat = default("created %p pk (%l)");

        @class(PacketSourceBase);
        @display("i=block/source");
        @signal[packetCreated](type=inet::Packet);
}
