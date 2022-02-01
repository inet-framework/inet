//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/dsdv/DsdvHelloSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/dsdv/DsdvHello_m.h"

namespace inet {

Register_Serializer(DsdvHello, DsdvHelloSerializer);

void DsdvHelloSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& dsdvHello = staticPtrCast<const DsdvHello>(chunk);
    stream.writeIpv4Address(dsdvHello->getSrcAddress());
    stream.writeUint32Be(dsdvHello->getSequencenumber());
    stream.writeIpv4Address(dsdvHello->getNextAddress());
    stream.writeUint32Be(dsdvHello->getHopdistance());
}

const Ptr<Chunk> DsdvHelloSerializer::deserialize(MemoryInputStream& stream) const
{
    auto dsdvHello = makeShared<DsdvHello>();
    dsdvHello->setSrcAddress(stream.readIpv4Address());
    dsdvHello->setSequencenumber(stream.readUint32Be());
    dsdvHello->setNextAddress(stream.readIpv4Address());
    dsdvHello->setHopdistance(stream.readUint32Be());
    dsdvHello->setChunkLength(B(16));
    return dsdvHello;
}

} // namespace inet

