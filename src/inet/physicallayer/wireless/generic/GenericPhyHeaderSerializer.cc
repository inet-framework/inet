//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/generic/GenericPhyHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/physicallayer/wireless/generic/GenericPhyHeader_m.h"

namespace inet {

Register_Serializer(GenericPhyHeader, GenericPhyHeaderSerializer);

void GenericPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& header = staticPtrCast<const GenericPhyHeader>(chunk);
    stream.writeUint16Be(b(header->getChunkLength()).get());
    // TODO KLUDGE this makes the serialized form dependent on the protocol ID
    stream.writeUint16Be(header->getPayloadProtocol()->getId());
    int64_t remainders = b(header->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("GenericPhyHeader length = %d bits is smaller than required %d bits", (int)b(header->getChunkLength()).get(), (int)b(stream.getLength() - startPosition).get());
    stream.writeBitRepeatedly(false, remainders);
}

const Ptr<Chunk> GenericPhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto header = makeShared<GenericPhyHeader>();
    b dataLength = b(stream.readUint16Be());
    header->setChunkLength(dataLength);
    // TODO KLUDGE this makes the serialized form dependent on the protocol ID
    header->setPayloadProtocol(Protocol::findProtocol(stream.readUint16Be()));
    b remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= b(0));
    stream.readBitRepeatedly(false, b(remainders).get());
    return header;
}

} // namespace inet

