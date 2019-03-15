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

import inet.physicallayer.contract.packetlevel.IAnalogModel;

//
// This analog model computes with scalar analog power representation. In other
// words, the signal power does not change over time or frequency, except for
// the boundaries of the signal.
//
module LoRaAnalogModel like IAnalogModel
{
    parameters:
        @display("i=block/tunnel");
        @class(inet::physicallayer::LoRaAnalogModel);
}
