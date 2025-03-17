//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ethernet;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPassivePacketSink;

//
// Provides a modular framework for building applications that communicate directly
// over Ethernet, bypassing the network and transport layers. Implements a flexible
// processing pipeline for Ethernet traffic with configurable components for traffic
// generation, processing, and consumption.
//
// The module consists of a packet source for generating outgoing traffic, a packet
// sink for processing incoming traffic, and an Ethernet socket interface that handles
// the communication. Additional processing elements like taggers and measurement
// modules can be inserted into the pipeline to implement custom traffic handling
// behaviors. This architecture enables direct communication at the data link layer,
// which is particularly useful for industrial automation protocols, real-time
// applications, and specialized communication scenarios that require minimal
// protocol overhead.
//
// @see ~EthernetSocketIo, ~EthernetSourceApp, ~EthernetSinkApp, ~EthernetSocket
//
module EthernetApp like IApp
{
    parameters:
        string interfaceTableModule; // Relative module path of the interface table
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
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
        io: EthernetSocketIo {
            parameters:
                @display("p=400,600");
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
