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
// This module processes incoming packets one by one in the order they arrive.
// First it classifies a packet according to the configured classifier function,
// then it generates tokens for the selected category in the configured response
// producer.
//
// @see ~ResponseProducer
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
