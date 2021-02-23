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

#include "inet/linklayer/ieee8021r/Ieee8021rTagHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee8021r/Ieee8021rTagHeader_m.h"

namespace inet {

Register_Serializer(Ieee8021rTagTpidHeader, Ieee8021rTagTpidHeaderSerializer);
Register_Serializer(Ieee8021rTagEpdHeader, Ieee8021rTagEpdHeaderSerializer);

void Ieee8021rTagTpidHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8021rTagTpidHeader>(chunk);
    stream.writeUint16Be(0xF1C1);
    stream.writeUint16Be(0);
    stream.writeUint16Be(header->getSequenceNumber());
}

const Ptr<Chunk> Ieee8021rTagTpidHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    const auto& header = makeShared<Ieee8021rTagTpidHeader>();
    auto tpid = stream.readUint16Be();
    if (tpid != 0xF1C1)
        header->markIncorrect();
    auto reserved = stream.readUint16Be();
    if (reserved != 0)
        header->markIncorrect();
    header->setSequenceNumber(stream.readUint16Be());
    return header;
}

void Ieee8021rTagEpdHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8021rTagEpdHeader>(chunk);
    stream.writeUint16Be(0);
    stream.writeUint16Be(header->getSequenceNumber());
    stream.writeUint16Be(header->getTypeOrLength());
}

const Ptr<Chunk> Ieee8021rTagEpdHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    const auto& header = makeShared<Ieee8021rTagEpdHeader>();
    auto reserved = stream.readUint16Be();
    if (reserved != 0)
        header->markIncorrect();
    header->setSequenceNumber(stream.readUint16Be());
    header->setTypeOrLength(stream.readUint16Be());
    return header;
}

} // namespace inet

