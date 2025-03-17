//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ipapp;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPassivePacketSink;

//
// Provides a modular framework for building IP-based applications using the queueing
// model architecture. Implements a flexible processing pipeline for IP traffic with
// configurable components for traffic generation, processing, and consumption. The
// architecture separates packet creation, processing, and reception into distinct
// components that can be independently customized.
//
// The module consists of a packet source for generating outgoing traffic, a packet
// sink for processing incoming traffic, and an IP socket interface that handles the
// communication. Additional processing elements like taggers and measurement modules
// can be inserted into the pipeline to implement custom traffic handling behaviors.
//
// This design enables the creation of various IP applications by simply configuring
// different component types without modifying the application framework itself. The
// parametric approach allows for extensive customization while maintaining a consistent
// interface, making it suitable as a base class for specialized IP applications.
//
// @see ~IpSocket, ~IpSocketIo, ~IActivePacketSource, ~IPassivePacketSink
//
module IpApp like IApp
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
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
        io: IpSocketIo {
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
