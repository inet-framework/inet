//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/mpls/MplsPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/mpls/MplsPacket_m.h"

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

