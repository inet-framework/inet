//
// Copyright (C) 2020 OpenSim Ltd.
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

package inet.queueing.meter;

import inet.queueing.contract.IPacketMeter;

module MultiPacketMeter like IPacketMeter
{
    parameters:
        int numMeters;
        @display("i=block/star");
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
    submodules:
        meter[numMeters]: <> like IPacketMeter {
            @display("p=150,150,row,150");
        }
    connections:
        in --> { @display("m=w"); } --> meter[0].in;
        for i=1..numMeters-1 {
            meter[i - 1].out --> meter[i].in;
        }
        meter[numMeters - 1].out --> { @display("m=e"); } --> out;
}
