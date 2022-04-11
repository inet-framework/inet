//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.protocol;

import inet.protocolelement.transceiver.PacketReceiver;
import inet.protocolelement.transceiver.PacketTransmitter;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

module ClientHost2
{
    parameters:
        @display("i=device/pc");
    gates:
        output out;
    submodules:
        app: ActivePacketSource {
            @display("p=150,100");
        }
        transmitter: PacketTransmitter {
            @display("p=150,200");
        }
    connections:
        app.out --> transmitter.in;
        transmitter.out --> out;
}

module ServerHost2
{
    parameters:
        @display("i=device/pc");
    gates:
        input in;
    submodules:
        app: PassivePacketSink {
            @display("p=150,100");
        }
        receiver: PacketReceiver {
            @display("p=150,200");
        }
    connections:
        in --> receiver.in;
        receiver.out --> app.in;
}

network Network2
{
    submodules:
        client: ClientHost2 {
            @display("p=100,100");
        }
        server: ServerHost2 {
            @display("p=200,100");
        }
    connections allowunconnected:
        client.out --> { datarate = 100Mbps; delay = 1us; } --> server.in;
}
