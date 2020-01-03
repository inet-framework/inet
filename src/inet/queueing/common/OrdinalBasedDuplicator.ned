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

package inet.queueing.common;

//
// This module duplicates packets based on the ordinal number of the packet in
// the received stream of packets.
//
simple OrdinalBasedDuplicator
{
    parameters:
        string duplicatesVector = default(""); // the duplicatesVector format is "<packetNumber>;<packetNumber>;...;"
                                               // packet numbers in ascending order e.g. "5;6;12;13;"
        @display("i=block/control");
    gates:
        input in;
        output out;
}

