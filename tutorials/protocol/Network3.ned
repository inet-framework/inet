//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.protocol;

import inet.protocolelement.transceiver.PacketReceiver;
import inet.protocolelement.transceiver.PacketTransmitter;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.InstantServer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

module ClientHost3
{
    parameters:
        @display("i=device/pc");
    gates:
        output out;
    submodules:
        app: ActivePacketSource {
            @display("p=150,100");
        }
        queue: PacketQueue {
            @display("p=150,200");
        }
        server: InstantServer {
            @display("p=150,300");
        }
        transmitter: PacketTransmitter {
            @display("p=150,400");
        }
    connections:
        app.out --> queue.in;
        queue.out --> server.in;
        server.out --> transmitter.in;
        transmitter.out --> out;
}

module ServerHost3
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

network Network3
{
    submodules:
        client: ClientHost3 {
            @display("p=100,100");
        }
        server: ServerHost3 {
            @display("p=200,100");
        }
    connections allowunconnected:
        client.out --> { datarate = 100Mbps; delay = 1us; } --> server.in;
}
