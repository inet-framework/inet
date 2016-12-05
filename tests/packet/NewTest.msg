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

cplusplus {{
#include "inet/common/packet/FieldsChunk.h"
}}

namespace inet;

class FieldsChunk extends cObject;
class SequenceChunk extends cObject;

enum Protocol
{
    Tcp = 0;
    Ip = 1;
    Ethernet = 2;
}

enum CrcMode
{
    CRC_DECLARED_CORRECT = 0;
    CRC_DECLARED_INCORRECT  = 1;
    CRC_COMPUTED = 2;
}

class ApplicationHeader extends FieldsChunk
{
    @str;
    int64_t chunkLength = 10;
    int16_t someData = -1;
}

class TcpHeader extends FieldsChunk
{
    @str;
    int64_t chunkLength = -1;
    int16_t lengthField = -1;
    int16_t srcPort = -1;
    int16_t destPort = -1;
    int16_t crc = 0;
    int crcMode @enum(CrcMode) = -1;
}

class IpHeader extends FieldsChunk
{
    @str;
    int64_t chunkLength = 20;
    int16_t protocol @enum(Protocol) = -1;
}

class EthernetHeader extends FieldsChunk
{
    @str;
    int64_t chunkLength = 14;
    int16_t protocol @enum(Protocol) = -1;
}

class EthernetTrailer extends FieldsChunk
{
    @str;
    int64_t chunkLength = 2;
    int16_t crc = 0;
    int crcMode @enum(CrcMode) = -1;
}
