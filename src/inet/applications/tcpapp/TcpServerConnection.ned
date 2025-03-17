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
// Provides a modular framework for handling individual TCP connections within server
// applications. Designed to be instantiated dynamically by ~TcpServerListener for each
// incoming client connection, this module implements a flexible processing pipeline
// for TCP traffic using the queueing model architecture.
//
// The module consists of configurable components including a packet source for generating
// outgoing traffic, a packet sink for processing incoming traffic, and a socket I/O
// interface that manages the TCP connection. Additional processing elements like taggers
// and measurement modules can be inserted into the pipeline to implement custom traffic
// handling behaviors.
//
// This architecture enables the creation of specialized server connection handlers by
// simply configuring different component types without modifying the connection framework
// itself. The parametric design allows for extensive customization while maintaining a
// consistent interface, making it suitable as a base class for various TCP server
// connection implementations.
//
// @see ~TcpServerApp, ~TcpServerListener, ~TcpServerSocketIo, ~IActivePacketSource, ~IPassivePacketSink
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
                @display("p=350,600");
        }
    connections allowunconnected:
        source.out --> tagger.in if exists(source);
        tagger.out --> measurementStarter.in;
        measurementStarter.out --> io.trafficIn;
        io.socketOut --> { @display("m=s"); } --> socketOut;

        socketIn --> { @display("m=s"); } --> io.socketIn;
        io.trafficOut --> measurementRecorder.in;
        measurementRecorder.out --> sink.in if exists(sink);
}
