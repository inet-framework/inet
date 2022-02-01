//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// This module repeatedly pulls packets from the connected packet provider and
// it pushes them into the connected packet consumer. The packets are processed
// in zero simulation time, and the process continues until the available number
// of tokens is not sufficient anymore.
//
simple TokenBasedServer extends PacketServerBase like IPacketServer
{
    parameters:
        double initialNumTokens = default(0); // initial number of tokens
        double maxNumTokens = default(nan); // maximum number of tokens
        volatile double tokenConsumptionPerPacket = default(1); // how many tokens are consumed per processed packet
        volatile double tokenConsumptionPerBit = default(0); // how many tokens are consumed per processed bit
        displayStringTextFormat = default("contains %n tk\nserved %p pk (%l)"); // determines display string text above the submodule
        @signal[tokensAdded](type=double);
        @signal[tokensRemoved](type=double);
        @signal[tokensDepleted](type=double);
        @statistic[addedTokens](title="added tokens"; source=tokensAdded; record=sum; unit=tk; interpolationmode=none);
        @statistic[removedTokens](title="removed tokens"; source=tokensRemoved; record=sum; unit=tk; interpolationmode=none);
        @statistic[numTokensDepleted](title="number of times tokens are depleted"; source=tokensDepleted; record=count; unit=tk; interpolationmode=none);
        @statistic[numTokens](title="number of tokens"; source=sum(tokensAdded) - sum(tokensRemoved); record=vector; unit=tk; interpolationmode=sample-hold);
        @class(TokenBasedServer);
}
