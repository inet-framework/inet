//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

enum EthernetControlOpCode
{
    ETHERNET_CONTROL_PAUSE = 1;
}

//
// This is a base class for all IEEE 802.3 Ethernet control frames defined the
// IEEE Std 802.3-2018, IEEE Standard for Ethernet.
//
class EthernetControlFrameBase extends FieldsChunk
{
    int opCode = -1;
}

//
// This class represents an IEEE 802.3 Ethernet MAC Pause frame as defined in
// the section 31.4.1 MAC Control frame format of the IEEE Std 802.3-2018, IEEE
// Standard for Ethernet.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-------------------------------+-------------------------------+
// |             OpCode            |           PauseTime           |
// +-------------------------------+-------------------------------+
//
class EthernetPauseFrame extends EthernetControlFrameBase
{
    chunkLength = B(4); // B(2) + B(2)
    opCode = ETHERNET_CONTROL_PAUSE;
    int pauseTime; // in 512 bit-time units
}
