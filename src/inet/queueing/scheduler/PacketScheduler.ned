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

package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// This module connects one packet collector to multiple packet providers. It
// can be pulled for packets by the connected packet collector. When this
// happens, the scheduler pulls a packet from one of its packet providers based
// on the configured scheduler function. The packet scheduler function takes a
// list of packet providers and returns an integer which in turn is used for
// determining the input gate.
//
simple PacketScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        string schedulerClass; // determines the input queue of the packet, the parameter must be the name of a C++ class which implements the IPacketSchedulerFunction interface and is registered via Register_Class
        @class(PacketScheduler);
}
