//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.tutorials.protocol;

import inet.protocol.acknowledgement.Resending;
import inet.protocol.checksum.FcsHeaderChecker;
import inet.protocol.checksum.FcsHeaderInserter;
import inet.protocol.common.InterPacketGap;
import inet.protocol.transceiver.PacketReceiver;
import inet.protocol.transceiver.PacketTransmitter;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.InstantServer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

module ClientHost6
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
        resending: Resending {
            @display("p=150,400");
        }
        fcsInserter: FcsHeaderInserter {
            @display("p=150,500");
        }
        interPacketGap: InterPacketGap {
            @display("p=150,600");
        }
        transmitter: PacketTransmitter {
            @display("p=150,700");
        }
    connections:
        app.out --> queue.in;
        queue.out --> server.in;
        server.out --> resending.in;
        resending.out --> fcsInserter.in;
        fcsInserter.out --> interPacketGap.in;
        interPacketGap.out --> transmitter.in;
        transmitter.out --> out;
}

module ServerHost6
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

network Network6
{
    submodules:
        client: ClientHost6 {
            @display("p=100,100");
        }
        server: ServerHost6 {
            @display("p=200,100");
        }
    connections allowunconnected:
        client.out --> { delay = 1us; ber = 1E-5; } --> server.in;
}
