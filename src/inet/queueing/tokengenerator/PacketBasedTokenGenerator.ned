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
// Generates tokens into the configured ~TokenBasedServer for each
// consumed packet.
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
