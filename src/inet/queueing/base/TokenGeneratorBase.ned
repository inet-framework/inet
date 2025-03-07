//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for token generator components in the queueing framework.
// Token generators create abstract tokens that are used to control packet
// processing in token-based modules such as servers, meters, and classifiers.
//
// Token-based processing is a fundamental concept in traffic management
// and quality of service implementations, allowing for:
// - Rate limiting and traffic shaping
// - Bandwidth allocation and reservation
// - Prioritization of traffic flows
// - Implementation of leaky bucket and token bucket algorithms
//
// Key features:
// - Generates tokens and adds them to a connected token storage
// - Supports various token generation patterns and rates
// - Collects statistics on token creation events
// - Provides the foundation for implementing traffic control mechanisms
//
// Derived modules can implement specific token generation strategies
// by defining when and how many tokens are created and added to the storage.
//
// @see ~TokenBasedServer, ~TokenBucketMeter, ~TokenBucketClassifier
//
simple TokenGeneratorBase extends PacketProcessorBase
{
    parameters:
        string storageModule; // Module path of the token storage where the tokens are generated via C++ method call
        displayStringTextFormat = default("generated %t tk\nto %s"); // Determines display string text above the submodule
        @class(TokenGeneratorBase);
        @display("i=block/control");
        @signal[tokensCreated](type=double);
        // the statistical value is the number of times tokens were created
        @statistic[tokensCreated](title="tokens created"; source=count(tokensCreated); record=count; unit=tk);
        // the statistical value is the number of created tokens
        @statistic[numTokensCreated](title="number of created tokens"; source=tokensCreated; record=sum,vector; unit=tk; interpolationmode=none);
}
