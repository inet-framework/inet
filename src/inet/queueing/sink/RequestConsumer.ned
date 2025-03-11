//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPassivePacketSink;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;

//
// Implements a request-response pattern for packet processing. Acts as the
// consumer side of a request-response pair, working in conjunction with a
// ResponseProducer to model bidirectional communication patterns.
//
// Key features:
// - Processes incoming request packets sequentially
// - Classifies requests into different categories using a configurable classifier
// - Generates tokens for the appropriate response category in the paired ResponseProducer
// - Controls processing rate through a token-based server
// - Supports multiple request/response types through the numKind parameter
//
// Operation mechanism:
// 1. Incoming request packets are stored in a queue
// 2. The server processes one packet at a time (controlled by tokens)
// 3. The classifier determines the request type/category
// 4. The corresponding token generator creates tokens in the ResponseProducer
// 5. These tokens enable the ResponseProducer to generate appropriate responses
//
// Applications:
// - Client-server communication modeling
// - Request-response protocol simulation
// - Service-oriented architecture simulation
// - API call modeling
// - Transaction processing systems
//
// The RequestConsumer is particularly useful in scenarios where:
// - Different types of requests require different types of responses
// - Request processing needs to be rate-limited
// - The system needs to model realistic request handling with queuing
// - Response generation depends on request classification
//
// Typically used with a ResponseProducer module that generates the corresponding
// responses based on the tokens produced by this module.
//
// @see ~ResponseProducer, ~TokenBasedServer, ~PacketBasedTokenGenerator
//
module RequestConsumer like IPassivePacketSink
{
    parameters:
        int numKind;
        string responseProducerModule = default("^.source");
        @display("i=block/sink");
    gates:
        input in;
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=200,100");
        }
        server: <default("TokenBasedServer")> like IPacketServer {
            initialNumTokens = default(1);
            @display("p=200,225");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=200,350");
        }
        tokenGenerator[numKind]: <default("PacketBasedTokenGenerator")> like IPassivePacketSink {
            storageModule = default("^." + parent.responseProducerModule + ".server[" + string(index) + "]");
            @display("p=200,475,row,300");
        }
    connections:
        in --> queue.in;
        queue.out --> server.in;
        server.out --> classifier.in;
        for i=0..sizeof(tokenGenerator) - 1 {
            classifier.out++ --> tokenGenerator[i].in;
        }
}
