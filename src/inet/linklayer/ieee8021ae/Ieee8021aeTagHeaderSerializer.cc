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

#include "inet/linklayer/ieee8021ae/Ieee8021aeTagHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee8021ae/Ieee8021aeTagHeader_m.h"

namespace inet {

Register_Serializer(Ieee8021aeTagTpidHeader, Ieee8021aeTagTpidHeaderSerializer);
Register_Serializer(Ieee8021aeTagEpdHeader, Ieee8021aeTagEpdHeaderSerializer);

void Ieee8021aeTagTpidHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8021aeTagTpidHeader>(chunk);
    stream.writeUint16Be(0x88E5);
    stream.writeByte(header->getTciAn());
    stream.writeByte(header->getSl());
    stream.writeUint32Be(header->getPn());
    // stream.writeUint64Be(header->getSci());
}

const Ptr<Chunk> Ieee8021aeTagTpidHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    const auto& header = makeShared<Ieee8021aeTagTpidHeader>();
    auto tpid = stream.readUint16Be();
    if (tpid != 0x66E5)
        header->markIncorrect();
    header->setTciAn(stream.readByte());
    header->setSl(stream.readByte());
    header->setPn(stream.readUint32Be());
//    header->setSci(stream.readUint64Be());
    return header;
}

void Ieee8021aeTagEpdHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8021aeTagEpdHeader>(chunk);
    stream.writeByte(header->getTciAn());
    stream.writeByte(header->getSl());
    stream.writeUint32Be(header->getPn());
//    stream.writeUint64Be(header->getSci());
    stream.writeUint16Be(header->getTypeOrLength());
}

const Ptr<Chunk> Ieee8021aeTagEpdHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    const auto& header = makeShared<Ieee8021aeTagEpdHeader>();
    header->setTciAn(stream.readByte());
    header->setSl(stream.readByte());
    header->setPn(stream.readUint32Be());
//    header->setSci(stream.readUint64Be());
    header->setTypeOrLength(stream.readUint16Be());
    return header;
}

} // namespace inet

