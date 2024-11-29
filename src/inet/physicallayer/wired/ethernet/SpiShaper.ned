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

import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;

//
// This module models a Serial Peripheral Interface (SPI) by connecting a packet
// queue and a packet server. The SPI is characterized by the clock frequency
// (speed). The packet server processing delay is calculated from the speed
// parameter and the packet length.
//
// @see ~SpiDelayer
//
module SpiShaper like ISpi
{
    parameters:
        double speed @unit(MHz);
        @display("i=block/mac");
    gates:
        input in;
        output out;
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=150,150");
        }
        server: <default("PacketServer")> like IPacketServer {
            processingTime = default(0s);
            processingBitrate = default(min(10, dropUnit(parent.speed)) * 1Mbps);
            @display("p=150,300");
        }
    connections:
        in --> { @display("m=n"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=s"); } --> out;
}
