//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.protocol;

import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

module ClientHost1
{
    parameters:
        @display("i=device/pc");
    gates:
        output out;
    submodules:
        app: ActivePacketSource {
            @display("p=100,100");
        }
    connections:
        app.out --> { @display("m=s"); } --> out;
}

module ServerHost1
{
    parameters:
        @display("i=device/pc");
    gates:
        input in;
    submodules:
        app: PassivePacketSink {
            @display("p=100,100");
        }
    connections:
        in --> { @display("m=s"); } --> app.in;
}

network Network1
{
    submodules:
        client: ClientHost1 {
            @display("p=100,100");
        }
        server: ServerHost1 {
            @display("p=200,100");
        }
    connections allowunconnected:
        client.out --> { delay = 1us; } --> server.in;
}
