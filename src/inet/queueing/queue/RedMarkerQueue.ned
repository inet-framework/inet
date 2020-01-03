//
// Copyright (C) 2019 Marcel Marek
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

package inet.queueing.queue;

import inet.queueing.contract.IPacketQueue;
import inet.queueing.filter.RedDropper;

//
// Combines a packet queue with random early detection (RED) dropper.
//
module RedDropperQueue extends CompoundPacketQueue
{
    submodules:
        red: RedDropper {
            @display("p=100,100");
        }
        fifo: <default("PacketQueue")> like IPacketQueue {
            @display("p=100,200");
        }
    connections:
        in --> { @display("m=n"); } --> red.in;
        red.out --> fifo.in;
        fifo.out --> { @display("m=s"); } --> out;
}
