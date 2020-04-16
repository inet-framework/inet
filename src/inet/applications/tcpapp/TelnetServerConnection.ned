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

package inet.applications.tcpapp;

import inet.applications.contract.IApp;
import inet.queueing.common.PacketCloner;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.IPassivePacketSink;
import inet.queueing.contract.IPassivePacketSource;
import inet.queueing.tokengenerator.PacketBasedTokenGenerator;

//
// This module contains a configurable pre-composed telnet traffic source and
// traffic sink as part of a telnet server application.
//
// @see ~TelnetClientApp, ~TelnetServerApp
//
module TelnetServerConnection like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        cloner: PacketCloner {
            parameters:
                @display("p=300,200");
        }
        responseProvider: <default("PassivePacketSource")> like IPassivePacketSource {
            parameters:
                @display("p=100,100");
        }
        responseServer: <default("TokenBasedServer")> like IPacketServer {
            parameters:
                @display("p=100,200");
        }
        multiplexer: PacketMultiplexer {
            parameters:
                @display("p=200,300");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            parameters:
                classifierClass = default("inet::queueing::PacketCharacterOrEnterClassifier");
                @display("p=500,200");
        }
        characterConsumer: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
                @display("p=400,300");
        }
        enterTokenGenerator: PacketBasedTokenGenerator {
            parameters:
                serverModule = default("^.responseServer");
                @display("p=600,300");
        }
        io: TcpServerSocketIo {
            parameters:
                @display("p=300,400");
        }
    connections:
        io.trafficOut --> cloner.in;
        cloner.out++ --> classifier.in;
        cloner.out++ --> multiplexer.in++;
        responseProvider.out --> responseServer.in;
        responseServer.out --> multiplexer.in++;
        classifier.out++ --> characterConsumer.in;
        classifier.out++ --> enterTokenGenerator.in;
        multiplexer.out --> { @display("m=s"); } --> io.trafficIn;
        io.socketOut --> { @display("m=s"); } --> socketOut;
        socketIn --> { @display("m=s"); } --> io.socketIn;
}
