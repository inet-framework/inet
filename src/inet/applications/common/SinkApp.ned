//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.common;

import inet.applications.contract.IApp;
import inet.queueing.contract.IPassivePacketSink;

//
// This module implements a generic sink application.
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

