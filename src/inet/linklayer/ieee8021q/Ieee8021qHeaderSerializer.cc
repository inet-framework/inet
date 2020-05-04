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

#include <algorithm>
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee8021q/Ieee8021qHeader_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qHeaderSerializer.h"

namespace inet {

Register_Serializer(Ieee8021qHeader, Ieee8021qHeaderSerializer);

void Ieee8021qHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8021qHeader>(chunk);
    stream.writeUint16Be(header->getTypeOrLength());
    stream.writeUint16Be((header->getVid() & 0xFFF) |
                         ((header->getPcp() & 7) << 13) |
                         (header->getDe() ? 0x1000 : 0));
}

const Ptr<Chunk> Ieee8021qHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Ieee8021qHeader> header = makeShared<Ieee8021qHeader>();
    header->setTypeOrLength(stream.readUint16Be());
    uint16_t qtagValue = stream.readUint16Be();
    header->setVid(qtagValue & 0xFFF);
    header->setPcp((qtagValue >> 13) & 7);
    header->setDe((qtagValue & 0x1000) != 0);
    return header;
}


} // namespace inet

