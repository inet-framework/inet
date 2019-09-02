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

package inet.common.queueing;

import inet.common.queueing.contract.IPacketProducer;

module ResponseProducer like IPacketProducer
{
    parameters:
        int numKind;
        @display("i=block/source");
    gates:
        output out;
    submodules:
        provider[numKind]: PacketProvider {
            @display("p=200,100,row,200");
        }
        server[numKind]: TokenBasedServer {
            tokenProductionInterval = default(0s);
            @display("p=200,200,row,200");
        }
        multiplexer: PacketMultiplexer {
            @display("p=200,300");
        }
    connections:
        for i=0..sizeof(provider) - 1 {
            provider[i].out --> server[i].in;
            server[i].out --> multiplexer.in++;
        }
        multiplexer.out --> { @display("m=s"); } --> out;
}
