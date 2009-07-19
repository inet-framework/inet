//
// Copyright (C) 2005 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.networklayer.queue;

//
// Prototype for per-NIC output queues. Concrete queues can implement
// drop-tail, RED etc. policy.
//
// Interface to Layer 2 modules: The C++ class implementing the queue
// must implement (i.e. subclass from) IPassiveQueue, and should
// send a packet whenever the L2 module asks for one by calling the
// requestPacket() method.
//
// @see DropTailQueue, DropTailQoSQueue, REDQueue
//
moduleinterface OutputQueue
{
    parameters:
        @display("i=block/queue");
    gates:
        input in;
        output out;
}

