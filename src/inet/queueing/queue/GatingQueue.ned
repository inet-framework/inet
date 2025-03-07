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
// Queue with a periodic gate that controls when packets can be transmitted.
// It is a key component for implementing Time-Sensitive Networking (TSN)
// and other time-scheduled communication systems.
//
// The GatingQueue consists of:
// - A standard packet queue that stores packets waiting for transmission
// - A periodic gate that opens and closes according to a configured schedule
//
// The periodic gate controls the flow of packets from the queue to the output,
// allowing packets to be transmitted only during specific time windows. This
// enables precise timing control for packet transmissions, which is essential
// for deterministic networking applications.
//
// The bitrate parameter specifies the transmission rate when the gate is open,
// and the extraLength parameter accounts for additional overhead (e.g., Ethernet PHY header).
//
// @see ~PeriodicGate, ~PacketQueue, ~GatingPriorityQueue
//
module GatingQueue extends CompoundPacketQueueBase
{
    parameters:
        double bitrate @unit(bps);
        *.bitrate = default(this.bitrate);
        *.extraLength = default(64b); // Overhead for Ethernet PHY header
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
