//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.server.TokenBasedServer;
import inet.queueing.tokengenerator.SignalBasedTokenGenerator;

//
// Implements the producer side of a request-response pattern for packet
// processing. Works in conjunction with a RequestConsumer to model bidirectional
// communication patterns where responses are generated based on received requests.
//
// Key features:
// - Supports multiple response types through separate packet providers
// - Token-based activation mechanism for precise response timing
// - Configurable packet characteristics for each response type
// - Signal-based coordination with the RequestConsumer
// - Multiplexes different response streams into a single output
//
// Operation mechanism:
// 1. The RequestConsumer processes incoming request packets
// 2. Based on the request type, it generates tokens for the appropriate server
// 3. The tokenGenerator monitors these tokens and activates the corresponding server
// 4. The activated server pulls packets from its provider
// 5. The multiplexer combines all response streams into a single output
//
// Typically used with a RequestConsumer module that processes incoming requests
// and triggers the appropriate response generation through token signaling.
//
// @see ~RequestConsumer, ~TokenBasedServer, ~SignalBasedTokenGenerator
//
module ResponseProducer like IActivePacketSource
{
    parameters:
        int numKind;
        string requestConsumerModule = default("^.sink");
        @display("i=block/source");
    gates:
        output out;
    submodules:
        provider[numKind]: PassivePacketSource {
            @display("p=300,100,row,200");
        }
        server[numKind]: TokenBasedServer {
            @display("p=300,225,row,200");
        }
        multiplexer: PacketMultiplexer {
            @display("p=300,350");
        }
        tokenGenerator: SignalBasedTokenGenerator {
            storageModule = default("^." + parent.requestConsumerModule + ".server");
            signals = default("tokensDepleted");
            @display("p=100,100");
        }
    connections:
        for i=0..sizeof(provider) - 1 {
            provider[i].out --> server[i].in;
            server[i].out --> multiplexer.in++;
        }
        multiplexer.out --> { @display("m=s"); } --> out;
}
