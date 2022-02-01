//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module generates tokens into the configured ~TokenBasedServer for each
// consumed packet.
//
simple PacketBasedTokenGenerator extends TokenGeneratorBase like IPassivePacketSink
{
    parameters:
        volatile double numTokensPerPacket = default(1); // number of tokens per consumed packet
        volatile double numTokensPerBit = default(0); // number of tokens per consumed bit
        displayStringTextFormat = default("processed %p pk (%l)\ngenerated %t tk\nto %s"); // %p number of packet, %l total length
        @class(PacketBasedTokenGenerator);
    gates:
        input in;
}
