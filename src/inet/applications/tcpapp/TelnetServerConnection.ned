//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
                storageModule = default("^.responseServer");
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
