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

import inet.protocol.transceiver.PacketReceiver;
import inet.protocol.transceiver.PacketTransmitter;
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
        client.out --> {  delay = 1us; } --> server.in;
}
