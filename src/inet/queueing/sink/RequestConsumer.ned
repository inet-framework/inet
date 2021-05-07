//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
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
            serverModule = default("^." + parent.responseProducerModule + ".server[" + string(index) + "]");
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
