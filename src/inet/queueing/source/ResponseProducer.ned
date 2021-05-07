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

package inet.queueing.source;

import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.server.TokenBasedServer;
import inet.queueing.tokengenerator.SignalBasedTokenGenerator;

//
// This module is capable of generating several different streams of packets,
// one after the other. For each traffic category, the packets are produced by
// a separate token based server. The traffic stream can be started by adding
// tokens the corresponding server module. The packet data, packet length, and
// timing distributions can be configured in the corresponding packet provider.
//
// @see ~RequestConsumer
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
            serverModule = default("^." + parent.requestConsumerModule + ".server");
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
