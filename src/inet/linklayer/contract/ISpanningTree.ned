//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.contract;

//
// Module interface for Spanning Tree protocols
//
moduleinterface ISpanningTree
{
    parameters:
        @display("i=block/network2");
        double helloTime @unit("s");
        double forwardDelay @unit("s");
        double maxAge @unit("s");
        int bridgePriority;
        bool visualize;
    gates:
        input relayIn;
        output relayOut;
}
