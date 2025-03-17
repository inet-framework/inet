//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPassivePacketSink;

//
// Provides a highly modular and customizable TCP client application framework based
// on the queueing model. Implements a flexible pipeline architecture where traffic
// generation, packet processing, and measurement components can be independently
// configured and connected. The modular design allows for creating diverse client
// behaviors by simply replacing or reconfiguring the component modules.
//
// The architecture consists of a packet source that generates traffic, optional
// processing stages (tagger, measurement starter), a TCP socket interface module
// that handles the connection, and a processing pipeline for received data
// (measurement recorder, sink). Each component in the pipeline can be customized
// or replaced to implement specific application requirements, enabling the
// simulation of various client behaviors from simple data transfer to complex
// application protocols.
//
// @see ~TcpServerApp, ~TcpClientSocketIo, ~IActivePacketSource, ~IPassivePacketSink
//
module TcpClientApp like IApp
{
    parameters:
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        source: <default("ActivePacketSource")> like IActivePacketSource if typename != "" {
            parameters:
                @display("p=200,150");
        }
        tagger: <default("")> like IPacketFlow {
            parameters:
                @display("p=200,300");
        }
        measurementStarter: <default("")> like IPacketFlow {
            parameters:
                @display("p=200,450");
        }
        sink: <default("PassivePacketSink")> like IPassivePacketSink if typename != "" {
            parameters:
                @display("p=500,150");
        }
        measurementRecorder: <default("")> like IPacketFlow {
            parameters:
                @display("p=500,450");
        }
        io: TcpClientSocketIo {
            parameters:
                @display("p=350,600");
        }
    connections allowunconnected:
        source.out --> tagger.in if exists(source);
        tagger.out --> measurementStarter.in;
        measurementStarter.out --> io.trafficIn;
        io.socketOut --> socketOut;

        socketIn --> io.socketIn;
        io.trafficOut --> measurementRecorder.in;
        measurementRecorder.out --> sink.in if exists(sink);
}
