//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.tcpapp;

//
// Accepts any number of incoming TCP connections, and discards whatever
// arrives on them. Compatible with both ~Ipv4 and ~Ipv6.
//
module TcpSinkApp extends TcpServerHostApp
{
    parameters:
        serverThreadModuleType = default("inet.applications.tcpapp.TcpSinkAppThread");
        bool usingTcpWithRead = default(false);
        volatile int readSize @unit(B) = default(-1B);    // used only with usingTcpWithRead==true 
        volatile double readDelay @unit(s) = default(-1s);    // used only with usingTcpWithRead==true, negative delay means immediatelly send a read command, otherwise schedule timer with >= 0 delay for waiting to send a read command  
        @class(TcpSinkApp);
        @display("i=block/sink");
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none; checkSignals=false);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none; checkSignals=false);
}

simple TcpSinkAppThread
{
    parameters:
        @display("i=block/sink");
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
}

