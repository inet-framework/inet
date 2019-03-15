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

package inet.lora.loraphy;

import inet.physicallayer.pathloss.FreeSpacePathLoss;

module LoRaPathLossOulu extends FreeSpacePathLoss
{
    parameters:
        //parameters taken from paper about measurements in Oulu
        double d0 = default(1000m) @unit(m);
        double n = default(2.32);
        double B = default(128.95);
        double sigma = default(7.8);
        double antennaGain = default(2);
        @class(inet::physicallayer::LoRaPathLossOulu);
}
