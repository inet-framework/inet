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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/protocol/checksum/header/CrcHeader_m.h"
#include "inet/protocol/checksum/serializer/CrcHeaderSerializer.h"

namespace inet {

Register_Serializer(CrcHeader, CrcHeaderSerializer);

void CrcHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& crcHeader = staticPtrCast<const CrcHeader>(chunk);
    auto crcMode = crcHeader->getCrcMode();
    if (crcMode != CRC_DISABLED && crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize CRC header without turned off or properly computed CRC, try changing the value of crcMode parameter for CrcHeaderInserter");
    stream.writeUint16Be(crcHeader->getCrc());
}

const Ptr<Chunk> CrcHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto crcHeader = makeShared<CrcHeader>();
    auto crc = stream.readUint16Be();
    crcHeader->setCrc(crc);
    crcHeader->setCrcMode(crc == 0 ? CRC_DISABLED : CRC_COMPUTED);
    return crcHeader;
}

} // namespace inet

