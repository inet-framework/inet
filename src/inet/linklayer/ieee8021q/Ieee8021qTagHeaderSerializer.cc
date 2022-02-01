//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qTagHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Register_Serializer(Ieee8021qTagTpidHeader, Ieee8021qTagTpidHeaderSerializer);
Register_Serializer(Ieee8021qTagEpdHeader, Ieee8021qTagEpdHeaderSerializer);

void Ieee8021qTagTpidHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8021qTagTpidHeader>(chunk);
    stream.writeUint16Be(header->getTpid());
    stream.writeUint16Be((header->getVid() & 0xFFF) |
                         ((header->getPcp() & 7) << 13) |
                         (header->getDei() ? 0x1000 : 0));
}

const Ptr<Chunk> Ieee8021qTagTpidHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    const auto& header = makeShared<Ieee8021qTagTpidHeader>();
    header->setTpid(stream.readUint16Be());
    uint16_t value = stream.readUint16Be();
    header->setVid(value & 0xFFF);
    header->setPcp((value >> 13) & 7);
    header->setDei((value & 0x1000) != 0);
    return header;
}

void Ieee8021qTagEpdHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8021qTagEpdHeader>(chunk);
    stream.writeUint16Be((header->getVid() & 0xFFF) |
                         ((header->getPcp() & 7) << 13) |
                         (header->getDei() ? 0x1000 : 0));
    stream.writeUint16Be(header->getTypeOrLength());
}

const Ptr<Chunk> Ieee8021qTagEpdHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    const auto& header = makeShared<Ieee8021qTagEpdHeader>();
    uint16_t value = stream.readUint16Be();
    header->setVid(value & 0xFFF);
    header->setPcp((value >> 13) & 7);
    header->setDei((value & 0x1000) != 0);
    header->setTypeOrLength(stream.readUint16Be());
    return header;
}

} // namespace inet

