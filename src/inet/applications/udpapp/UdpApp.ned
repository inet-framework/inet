//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPassivePacketSink;

//
// This module generates traffic for a UDP application. The traffic source and
// traffic sink modules can be built from queueing model elements.
//
module UdpApp like IApp
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
        io: UdpSocketIo {
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
