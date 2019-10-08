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

package inet.queueing;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketConsumer;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;

module RequestConsumer like IPacketConsumer
{
    parameters:
        int numKind;
        string responseProducerModule = default("^.sink");
        @display("i=block/sink");
    gates:
        input in;
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=200,100");
        }
        server: <default("PacketServer")> like IPacketServer {
            processingTime = default(0s);
            @display("p=200,200");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=200,300");
        }
        tokenGenerator[numKind]: <default("PacketBasedTokenGenerator")> like IPacketConsumer {
            serverModule = default("^." + responseProducerModule + ".server[" + string(index) + "]");
            @display("p=200,400,row,200");
        }
    connections:
        in --> queue.in;
        queue.out --> server.in;
        server.out --> classifier.in;
        for i=0..sizeof(tokenGenerator) - 1 {
            classifier.out++ --> tokenGenerator[i].in;
        }
}
