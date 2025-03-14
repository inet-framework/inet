//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;
import inet.queueing.contract.IPassivePacketSink;
import inet.queueing.contract.ITokenGenerator;

//
// Generates tokens for a token-based server based on incoming packets.
// Acts as a packet sink that consumes packets and converts them into tokens,
// which can then be used to control the processing rate of other modules.
//
// Key features:
// - Generates tokens based on packet count and/or packet size
// - Configurable token generation rates per packet and per bit
// - Can generate a fixed or variable number of tokens per packet
// - Connects to a token storage module (typically a TokenBasedServer)
// - Implements both ITokenGenerator and IPassivePacketSink interfaces
//
// Operation mechanism:
// 1. The module receives packets through its input gate
// 2. For each packet, it generates tokens based on:
//    - A fixed or variable number of tokens per packet (numTokensPerPacket)
//    - A fixed or variable number of tokens per bit (numTokensPerBit)
// 3. The generated tokens are added to the configured token storage module
// 4. The packet is consumed (removed from the simulation)
//
// @see ~TokenBasedServer, ~QueueBasedTokenGenerator, ~TimeBasedTokenGenerator, ~SignalBasedTokenGenerator
//
simple PacketBasedTokenGenerator extends TokenGeneratorBase like ITokenGenerator, IPassivePacketSink
{
    parameters:
        volatile double numTokensPerPacket = default(1); // Number of tokens per consumed packet
        volatile double numTokensPerBit = default(0); // Number of tokens per consumed bit
        displayStringTextFormat = default("processed %p pk (%l)\ngenerated %t tk\nto %s"); // %p number of packets, %l total length
        @class(PacketBasedTokenGenerator);
    gates:
        input in;
}
