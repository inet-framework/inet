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
#include "inet/linklayer/ieee8021q/Ieee8021qHeader_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qHeaderSerializer.h"

namespace inet {

Register_Serializer(Ieee8021qHeader, Ieee8021qHeaderSerializer);

void Ieee8021qHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ieee8021qHeader = staticPtrCast<const Ieee8021qHeader>(chunk);
    stream.writeUint16Be(0x8100);
    stream.writeNBitsOfUint64Be(ieee8021qHeader->getPcp(), 3);
    stream.writeBit(ieee8021qHeader->getDe());
    stream.writeNBitsOfUint64Be(ieee8021qHeader->getVid(), 12);
}

const Ptr<Chunk> Ieee8021qHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ieee8021qHeader = makeShared<Ieee8021qHeader>();
    stream.readUint16Be();
    ieee8021qHeader->setPcp(stream.readNBitsToUint64Be(3));
    ieee8021qHeader->setDe(stream.readBit());
    ieee8021qHeader->setVid(stream.readNBitsToUint64Be(12));
    return ieee8021qHeader;
}

} // namespace inet
