//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

package inet.linklayer.queue;

//
// This module selectively drops packets, based on the available
// buffer space of the queues attached to its output.
//
// The componenent has n input and n output gates (specified by
// the numGates parameter). Packets arrived at the ith input
// gate are transmitted on the ith output gate, or dropped.
//
// The module sums the buffer lengths of its outputs
// and if enqueuing a packet would exceed the configured
// capacities, then the packet will be dropped instead.
//
// Be attaching a ThresholdDropper to the input of a FIFO
// queue, you can compose a DropTailQueue. Shared buffer
// space can be modeled by attaching more FIFO queues
// to the output.
//
simple ThresholdDropper
{
    parameters:
        int numGates = default(1); // number of input and output gates
        int frameCapacity = default(-1); // if positive, then limits the sum of frames in output queues
        int byteCapacity = default(-1);  // if positive, then limits the sum of bytes in the output queues
        @display("i=block/downarrow");

    gates:
        input in[numGates];
        output out[numGates];
}
