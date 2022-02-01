//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskPhyHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskPhyHeader_m.h"

namespace inet {

Register_Serializer(UnitDiskPhyHeader, UnitDiskPhyHeaderSerializer);

void UnitDiskPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& header = staticPtrCast<const UnitDiskPhyHeader>(chunk);
    stream.writeUint16Be(b(header->getChunkLength()).get());
    stream.writeUint16Be(header->getPayloadProtocol()->getId());
    int64_t remainders = b(header->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("UnitDiskPhyHeader length = %d bits is smaller than required %d bits", (int)b(header->getChunkLength()).get(), (int)b(stream.getLength() - startPosition).get());
    stream.writeBitRepeatedly(false, remainders);
}

const Ptr<Chunk> UnitDiskPhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto header = makeShared<UnitDiskPhyHeader>();
    b dataLength = b(stream.readUint16Be());
    header->setChunkLength(dataLength);
    header->setPayloadProtocol(Protocol::findProtocol(stream.readUint16Be()));
    b remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= b(0));
    stream.readBitRepeatedly(false, b(remainders).get());
    return header;
}

} // namespace inet

