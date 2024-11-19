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

import inet.queueing.common.PacketDelayer;

//
// This module models the delay of sending a packet through a Serial Peripheral
// Interface (SPI). The SPI is characterized by the clock frequency (speed). The
// transmission delay is calculated from the speed parameter and the packet length.
//
// @see ~SpiShaper
//
simple SpiDelayer extends PacketDelayer like ISpi
{
    parameters:
        double speed @unit(MHz);
        delay = default(0s);
        bitrate = default(min(10, dropUnit(this.speed)) * 1Mbps);
        @display("i=block/mac");
}
