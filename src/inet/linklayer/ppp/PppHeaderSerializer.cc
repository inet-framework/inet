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

void PppHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pppHeader = staticPtrCast<const PppHeader>(chunk);
    stream.writeUint16Be(pppHeader->getProtocol());
}

const Ptr<Chunk> PppHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto pppHeader = makeShared<PppHeader>();
    pppHeader->setProtocol(stream.readUint16Be());
    return pppHeader;
}

} // namespace inet

