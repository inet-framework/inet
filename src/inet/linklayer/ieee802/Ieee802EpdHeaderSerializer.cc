//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee802/Ieee802EpdHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"

namespace inet {

Register_Serializer(Ieee802EpdHeader, Ieee802EpdHeaderSerializer);

void Ieee802EpdHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& llcHeader = CHK(dynamicPtrCast<const Ieee802EpdHeader>(chunk));
    stream.writeUint16Be(llcHeader->getEtherType());
}

const Ptr<Chunk> Ieee802EpdHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Ieee802EpdHeader> llcHeader = makeShared<Ieee802EpdHeader>();
    llcHeader->setEtherType(stream.readUint16Be());
    return llcHeader;
}

} // namespace inet

