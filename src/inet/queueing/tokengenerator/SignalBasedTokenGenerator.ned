//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;
import inet.queueing.contract.ITokenGenerator;

//
// This module generates tokens for a ~TokenBasedServer based on received signals.
//
simple SignalBasedTokenGenerator extends TokenGeneratorBase like ITokenGenerator
{
    parameters:
        string subscriptionModule = default("^");
        string signals;
        volatile double numTokens = default(1); // Number of tokens generated repeatedly
        @class(SignalBasedTokenGenerator);
        @display("i=block/control");
}
