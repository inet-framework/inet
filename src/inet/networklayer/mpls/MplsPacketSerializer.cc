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
#include "inet/networklayer/mpls/MplsPacket_m.h"
#include "inet/networklayer/mpls/MplsPacketSerializer.h"

namespace inet {

Register_Serializer(MplsHeader, MplsPacketSerializer);

void MplsPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& mplsHeader = staticPtrCast<const MplsHeader>(chunk);
    stream.writeNBitsOfUint64Be(mplsHeader->getLabel(), 20);
    stream.writeNBitsOfUint64Be(mplsHeader->getTc(), 3);
    stream.writeBit(mplsHeader->getS());
    stream.writeUint8(mplsHeader->getTtl());
}

const Ptr<Chunk> MplsPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto mplsHeader = makeShared<MplsHeader>();
    mplsHeader->setLabel(stream.readNBitsToUint64Be(20));
    mplsHeader->setTc(stream.readNBitsToUint64Be(3));
    mplsHeader->setS(stream.readBit());
    mplsHeader->setTtl(stream.readUint8());
    return mplsHeader;
}

} // namespace inet
