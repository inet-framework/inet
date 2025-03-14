//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// Resource-constrained packet server that processes packets based on token
// availability. This module models systems with limited processing resources
// that must be allocated for each packet.
//
// Operation mechanism:
// - Processes packets in zero simulation time when tokens are available
// - Each packet consumes tokens based on packet count and/or bit length
// - Processing continues until available tokens are insufficient
// - Processing resumes when new tokens become available
//
// The token consumption can be configured to be:
// - Fixed per packet (tokenConsumptionPerPacket)
// - Variable based on packet size (tokenConsumptionPerBit)
// - A combination of both fixed and variable components
//
// Tokens can be added externally by other modules to model resource
// replenishment, enabling complex resource allocation strategies.
//
// @see ~PacketServer, ~InstantServer, ~PreemptingServer
//
simple TokenBasedServer extends PacketServerBase like IPacketServer
{
    parameters:
        double initialNumTokens = default(0); // Initial number of tokens
        double maxNumTokens = default(nan); // Maximum number of tokens
        volatile double tokenConsumptionPerPacket = default(1); // How many tokens are consumed per processed packet
        volatile double tokenConsumptionPerBit = default(0); // How many tokens are consumed per processed bit
        displayStringTextFormat = default("contains %n tk\nserved %p pk (%l)"); // Determines display string text above the submodule
        @signal[tokensAdded](type=double);
        @signal[tokensRemoved](type=double);
        @signal[tokensDepleted](type=double);
        @statistic[addedTokens](title="added tokens"; source=tokensAdded; record=sum; unit=tk; interpolationmode=none);
        @statistic[removedTokens](title="removed tokens"; source=tokensRemoved; record=sum; unit=tk; interpolationmode=none);
        @statistic[numTokensDepleted](title="number of times tokens are depleted"; source=tokensDepleted; record=count; unit=tk; interpolationmode=none);
        @statistic[numTokens](title="number of tokens"; source=sum(tokensAdded) - sum(tokensRemoved); record=vector; unit=tk; interpolationmode=sample-hold);
        @class(TokenBasedServer);
}
