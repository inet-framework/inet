//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ppp/PppHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ppp/PppFrame_m.h"

namespace inet {

Register_Serializer(PppHeader, PppHeaderSerializer);
Register_Serializer(PppTrailer, PppTrailerSerializer);

void PppHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pppHeader = staticPtrCast<const PppHeader>(chunk);
    stream.writeUint8(pppHeader->getFlag());
    stream.writeUint8(pppHeader->getAddress());
    stream.writeUint8(pppHeader->getControl());
    stream.writeUint16Be(pppHeader->getProtocol());
}

const Ptr<Chunk> PppHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pppHeader = makeShared<PppHeader>();
    pppHeader->setFlag(stream.readUint8());
    pppHeader->setAddress(stream.readUint8());
    pppHeader->setControl(stream.readUint8());
    pppHeader->setProtocol(stream.readUint16Be());
    return pppHeader;
}

void PppTrailerSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pppTrailer = staticPtrCast<const PppTrailer>(chunk);
    stream.writeUint16Be(pppTrailer->getFcs());
//    stream.writeUint8(pppTrailer->getFlag()); //KLUDGE length is currently 2 bytes instead of 3 bytes
}

const Ptr<Chunk> PppTrailerSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pppTrailer = makeShared<PppTrailer>();
    pppTrailer->setFcs(stream.readUint16Be());
//    pppTrailer->setFlag(stream.readUint8()); //KLUDGE length is currently 2 bytes instead of 3 bytes
    return pppTrailer;
}

} // namespace inet

