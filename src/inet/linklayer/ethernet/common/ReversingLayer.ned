//
// Copyright (C) 2021 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.linklayer.ethernet.common;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.common.PacketMultiplexer;


module ReversingLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        fork: EthernetForker {
            @display("p=100,100;i=-;b=10,10,oval,grey,,1");
        }
        reverser: PacketDirectionReverser {
            @display("p=300,100");
        }
        join: PacketMultiplexer {
            @display("p=500,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> join.in++;
        join.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> fork.in;
        fork.out++ --> { @display("m=n"); } --> upperLayerOut;

        fork.out++ --> reverser.in;
        reverser.out --> join.in++;
}
