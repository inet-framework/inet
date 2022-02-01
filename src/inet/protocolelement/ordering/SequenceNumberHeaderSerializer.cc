//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/ordering/SequenceNumberHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Register_Serializer(SequenceNumberHeader, SequenceNumberHeaderSerializer);

void SequenceNumberHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& sequenceNumberHeader = staticPtrCast<const SequenceNumberHeader>(chunk);
    stream.writeUint16Be(sequenceNumberHeader->getSequenceNumber());
}

const Ptr<Chunk> SequenceNumberHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto sequenceNumberHeader = makeShared<SequenceNumberHeader>();
    sequenceNumberHeader->setSequenceNumber(stream.readUint16Be());
    return sequenceNumberHeader;
}

} // namespace inet

