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
// Generates traffic for a UDP application. The traffic source and
// traffic sink modules can be built from queueing model elements.
//
module UdpApp like IApp
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        string protocol = default("udp"); // Transport protocol: "udp" or "udplite"
        int sendCoverage = default(-1); // UDPLite send checksum coverage: -1=module default, 0=full, >=8=partial bytes
        int recvCoverage = default(-1); // UDPLite minimum acceptable recv coverage: -1=accept any, 0=accept any, >=8=minimum bytes
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
                protocol = default(parent.protocol);
                sendCoverage = default(parent.sendCoverage);
                recvCoverage = default(parent.recvCoverage);
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
