//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.protocol;

import inet.protocolelement.checksum.FcsHeaderChecker;
import inet.protocolelement.checksum.FcsHeaderInserter;
import inet.protocolelement.common.InterpacketGapInserter;
import inet.protocolelement.transceiver.PacketReceiver;
import inet.protocolelement.transceiver.PacketTransmitter;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.InstantServer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

module ClientHost5
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
        fcsInserter: FcsHeaderInserter {
            @display("p=150,400");
        }
        InterpacketGapInserter: InterpacketGapInserter {
            @display("p=150,500");
        }
        transmitter: PacketTransmitter {
            @display("p=150,600");
        }
    connections:
        app.out --> queue.in;
        queue.out --> server.in;
        server.out --> fcsInserter.in;
        fcsInserter.out --> InterpacketGapInserter.in;
        InterpacketGapInserter.out --> transmitter.in;
        transmitter.out --> out;
}

module ServerHost5
{
    parameters:
        @display("i=device/pc");
    gates:
        input in;
    submodules:
        app: PassivePacketSink {
            @display("p=150,100");
        }
        fcsChecker: FcsHeaderChecker {
            @display("p=150,200");
        }
        receiver: PacketReceiver {
            @display("p=150,300");
        }
    connections:
        in --> receiver.in;
        receiver.out --> fcsChecker.in;
        fcsChecker.out --> app.in;
}

network Network5
{
    submodules:
        client: ClientHost5 {
            @display("p=100,100");
        }
        server: ServerHost5 {
            @display("p=200,100");
        }
    connections allowunconnected:
        client.out --> { delay = 1us; ber = 1E-5; } --> server.in;
}
