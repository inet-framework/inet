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

//
// TODO generated message class
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.linklayer.common.MacAddress;

namespace inet::flora;

cplusplus {{
using namespace inet;
}}

class LoRaMacControlInfo {
    inet::MacAddress src;  // src DevAddr address (can be left empty when sending)
    inet::MacAddress dest; // dest DevAddr address

    double LoRaTP;
    inet::Hz LoRaCF;
    int LoRaSF;
    inet::Hz LoRaBW;
    int LoRaCR;
    bool LoRaUseHeader;
}
