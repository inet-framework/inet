//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

import inet.applications.contract.IApp;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.contract.IPassivePacketSink;

//
// Modular telnet client application that simulates interactive terminal behavior
// using a queueing-based architecture. Generates realistic keystroke patterns
// including regular characters, enter key presses, and think-time pauses to
// model human typing behavior in terminal sessions.
//
// Implements a sophisticated traffic generation model with three packet producers
// (for characters, enter keys, and pauses) and a Markov scheduler that creates
// realistic typing sequences. The character producer generates lowercase ASCII
// characters at configurable typing speeds, while the scheduler controls the
// transition between typing characters, pressing enter, and pausing to simulate
// user think time. All generated traffic is sent to a TCP server through the
// TcpClientSocketIo module, with received server responses handled by a
// configurable packet sink.
//
// @see ~TelnetServerApp, ~TelnetApp, ~TcpClientSocketIo, ~MarkovScheduler
//
module TelnetClientApp like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        characterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = intuniform(97, 122); // Lower case ASCII characters
                productionInterval = uniform(0.1s, 0.2s); // Typing speed between 5 and 10 characters per second
                @display("p=100,100");
        }
        enterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = 13; // Enter character
                productionInterval = 0.1s;
                @display("p=300,100");
        }
        emptyProducer: <default("EmptyPacketSource")> like IActivePacketSource {
            parameters:
                @display("p=500,100");
        }
        scheduler: <default("MarkovScheduler")> like IPacketScheduler {
            parameters:
                transitionProbabilities = "0 1 0 0 0 1 1 0 0"; // Character -> enter -> wait -> character
                waitIntervals = "uniform(0,3) 0 uniform(10,30)";
                @display("p=300,200");
        }
        consumer: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
                @display("p=100,200");
        }
        io: TcpClientSocketIo {
            parameters:
                @display("p=200,300");
        }
    connections:
        characterProducer.out --> scheduler.in++;
        enterProducer.out --> scheduler.in++;
        emptyProducer.out --> scheduler.in++;
        scheduler.out --> { @display("m=s"); } --> io.trafficIn;
        io.trafficOut --> { @display("m=s"); } --> consumer.in;
        io.socketOut --> { @display("m=s"); } --> socketOut;
        socketIn --> { @display("m=s"); } --> io.socketIn;
}
