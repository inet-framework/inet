//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;

//
// This module generates tokens for a ~TokenBasedServer repeatedly.
//
simple TimeBasedTokenGenerator extends TokenGeneratorBase
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        volatile double generationInterval @unit(s); // Time interval between two subsequent token generation events
        volatile double numTokens = default(1); // Number of tokens generated repeatedly
        @class(TimeBasedTokenGenerator);
        @display("i=block/control");
}
