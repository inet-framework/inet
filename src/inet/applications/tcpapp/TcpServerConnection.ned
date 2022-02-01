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
// This module is a generic TCP server connection. The traffic source and traffic
// sink modules can be built from queueing model elements.
//
// @see ~TcpServerApp
//
module TcpServerConnection like IApp
{
    parameters:
        @application;
        @display("i=block/cogwheel");
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
        io: TcpServerSocketIo {
            parameters:
                @display("p=150,300");
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
