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
#include "inet/physicallayer/unitdisk/UnitDiskPhyHeader_m.h"
#include "inet/physicallayer/unitdisk/UnitDiskPhyHeaderSerializer.h"

namespace inet {

Register_Serializer(UnitDiskPhyHeader, UnitDiskPhyHeaderSerializer);

void UnitDiskPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& header = staticPtrCast<const UnitDiskPhyHeader>(chunk);
    stream.writeUint16Be(b(header->getChunkLength()).get());
    stream.writeUint16Be(header->getPayloadProtocol()->getId());
    int64_t remainders = b(header->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("UnitDiskPhyHeader length = %d bits is smaller than required %d bits", (int)b(header->getChunkLength()).get(), (int)b(stream.getLength() - startPosition).get());
    stream.writeBitRepeatedly(false, remainders);
}

const Ptr<Chunk> UnitDiskPhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto header = makeShared<UnitDiskPhyHeader>();
    b dataLength = b(stream.readUint16Be());
    header->setChunkLength(dataLength);
    header->setPayloadProtocol(Protocol::findProtocol(stream.readUint16Be()));
    b remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= b(0));
    stream.readBitRepeatedly(false, b(remainders).get());
    return header;
}

} // namespace inet

