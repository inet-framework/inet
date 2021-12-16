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

package inet.queueing.filter;

import inet.queueing.contract.IPacketFilter;

//
// This module combines multiple packet filters into one.
//
module MultiPacketFilter like IPacketFilter
{
    parameters:
        int numFilters;
        @display("i=block/filter");
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
    submodules:
        filter[numFilters]: <> like IPacketFilter {
            @display("p=150,150,row,150");
        }
    connections:
        in --> { @display("m=w"); } --> filter[0].in;
        for i=1..numFilters-1 {
            filter[i - 1].out --> filter[i].in;
        }
        filter[numFilters - 1].out --> { @display("m=e"); } --> out;
}
