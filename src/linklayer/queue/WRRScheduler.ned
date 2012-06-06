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
// This module implements weighted round-robin scheduling.
//
// There is a weight associated with each input gate.
// Initially each input gate gets as many tokens as their
// weight. When a packet is requested then the first input
// gate is selected that has a packet and enough tokens,
// and one token is consumed. When there are waiting input
// packets, but there is no enough tokens to schedule one,
// then each token bucket is filled up.
//
// This module implements the IPassiveQueue C++ interface,
// therefore it can be used as the queue component of a NIC,
// and as the input of another scheduler.
//
simple WRRScheduler
{
    parameters:
        string weights;
        @display("i=block/server");

    gates:
        input in[];
        output out;
}
