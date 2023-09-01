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

package inet.physicallayer.wired.ethernet;

import inet.queueing.queue.PacketQueue;
import inet.queueing.server.PacketServer;

module SpiInterface
{
    parameters:
        double speed @unit(MHz);
        @display("i=block/mac");
    gates:
        input in;
        output out;
    submodules:
        queue: PacketQueue {
            @display("p=150,150");
        }
        server: PacketServer {
            processingTime = 0s;
            processingBitrate = min(10, dropUnit(parent.speed)) * 1Mbps;
            @display("p=150,300");
        }
    connections:
        in --> { @display("m=n"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=s"); } --> out;
}
