//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;
import inet.queueing.contract.ITokenGenerator;

//
// Generates tokens for a ~TokenBasedServer based on the observed state
// changes of a packet queue. Its primary purpose is to allow applications to
// produce traffic which completely utilizes a network interface. This is achieved
// by generating tokens whenever the observed packet queue becomes empty.
//
// Available display string text format directives:
//  %p - number of processed packets
//  %l - processed total packet length
//  %t - number of generated tokens
//
simple QueueBasedTokenGenerator extends TokenGeneratorBase like ITokenGenerator
{
    parameters:
        string queueModule; // Module path of the observed packet queue
        int minNumPackets = default(1);
        int minTotalLength @unit(b) = default(0 b);
        volatile double numTokens = default(1); // Number of tokens generated when the queue becomes empty
        @class(QueueBasedTokenGenerator);
        @display("i=block/control");
}
