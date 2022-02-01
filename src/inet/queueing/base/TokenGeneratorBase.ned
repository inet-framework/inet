//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various token generator modules.
//
simple TokenGeneratorBase extends PacketProcessorBase
{
    parameters:
        string storageModule; // module path of the token storage where the tokens are generated via C++ method call
        displayStringTextFormat = default("generated %t tk\nto %s"); // determines display string text above the submodule
        @class(TokenGeneratorBase);
        @display("i=block/control");
        @signal[tokensCreated](type=double);
        // the statistical value is the number of times tokens were created
        @statistic[tokensCreated](title="tokens created"; source=count(tokensCreated); record=count; unit=tk);
        // the statistical value is the number of created tokens
        @statistic[numTokensCreated](title="number of created tokens"; source=tokensCreated; record=sum,vector; unit=tk; interpolationmode=none);
}
