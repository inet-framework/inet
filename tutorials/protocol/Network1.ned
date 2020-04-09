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
        client.out --> {  delay = 1us; } --> server.in;
}
