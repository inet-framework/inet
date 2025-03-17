//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.common;

import inet.applications.contract.IApp;
import inet.queueing.contract.IPassivePacketSink;

//
// Provides a minimal, protocol-independent packet sink application that consumes
// and discards incoming packets from any transport protocol. Serves as a flexible
// endpoint for traffic generation scenarios where only the traffic reception
// behavior is relevant, without implementing any specific application-layer protocol.
//
// The module uses a configurable packet sink component that can be customized to
// implement different packet consumption behaviors. By default, it uses a passive
// packet sink that simply discards received packets, but it can be replaced with
// other sink implementations that provide statistics collection, conditional
// processing, or other specialized behaviors. Designed to be used in conjunction
// with various traffic sources for network performance testing and evaluation.
//
// @see ~SourceApp, ~IPassivePacketSink, ~PassivePacketSink
//
module SinkApp like IApp
{
    parameters:
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        sink: <default("PassivePacketSink")> like IPassivePacketSink {
            @display("p=150,150");
        }
    connections allowunconnected:
        socketIn --> sink.in;
}
