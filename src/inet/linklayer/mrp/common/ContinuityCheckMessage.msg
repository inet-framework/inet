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

//import inet.linklayer.common.MacAddress;
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// TODO generated message class
//

class continuityCheckMessage extends FieldsChunk {
    chunkLength = B(38);
    uint8_t mdLevel = 0;
    uint8_t OpCode = 1;
    uint8_t flags = 0x00000001; //RDI 1 bit,, Traffic 1 bit, Reserved 3 bit, CCM interval 3 bit
    uint8_t firstTLVoffset = 0;
    uint32_t sequenceNumber = 0;
    uint16_t endpointIdentifier = 1;
    uint8_t nameFormat = 4;
    uint8_t nameLength = 20;
    string messageName; //max 41 Bytes
    uint16_t shortNameFormat = 3;
    uint8_t shortNameLength = 2;
    uint16_t shortName = 0x0001;
    uint64_t ituInfo1 = 0;
    uint64_t ituInfo2 = 0;
    uint8_t endTlv = 0;  
}

message continuityCheckTimer
{
    int port;
}

message processDelayTimer
{
    int port;
    int field;
}