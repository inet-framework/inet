//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.
//

package inet.queueing.queue;

import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;

//
// This module implements a queue having its own periodic gate.
//
module GatingQueue extends CompoundPacketQueueBase
{
    parameters:
        double bitrate @unit(bps);
        *.bitrate = default(this.bitrate);
        *.extraLength = default(64b); // overhead for Ethernet PHY header
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=300,100,column,125");
        }
        gate: <default("PeriodicGate")> like IPacketGate {
            @display("p=700,100,column,125");
        }
    connections:
        in --> { @display("m=w"); } --> queue.in;
        queue.out --> gate.in;
        gate.out --> { @display("m=e"); } --> out;
}
