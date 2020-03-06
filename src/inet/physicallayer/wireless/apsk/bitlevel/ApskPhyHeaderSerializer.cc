//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/bitlevel/ApskPhyHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {
namespace physicallayer {

Register_Serializer(ApskPhyHeader, ApskPhyHeaderSerializer);

void ApskPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    auto startPosition = stream.getLength();
    const auto& phyHeader = staticPtrCast<const ApskPhyHeader>(chunk);
    stream.writeUint16Be(b(phyHeader->getHeaderLengthField()).get());
    stream.writeUint16Be(b(phyHeader->getPayloadLengthField()).get());
    //TODO write protocol

    b remainders = phyHeader->getChunkLength() - (stream.getLength() - startPosition);
    if (remainders < b(0))
        throw cRuntimeError("ApskPhyHeader length = %d smaller than required %d bytes", (int)B(phyHeader->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    uint8_t remainderbits = remainders.get() % 8;
    stream.writeByteRepeatedly('?', B(remainders - b(remainderbits)).get());
    stream.writeBitRepeatedly(false, remainderbits);
}

const Ptr<Chunk> ApskPhyHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto phyHeader = makeShared<ApskPhyHeader>();
    b headerLength = b(stream.readUint16Be());
    phyHeader->setHeaderLengthField(headerLength);
    phyHeader->setChunkLength(headerLength);
    phyHeader->setPayloadLengthField(b(stream.readUint16Be()));
    //TODO read protocol

    b curLength = stream.getPosition() - startPosition;
    b remainders = headerLength - curLength;
    if (remainders < b(0)) {
        phyHeader->markIncorrect();
    }
    else {
        uint8_t remainderbits = remainders.get() % 8;
        stream.readByteRepeatedly('?', B(remainders - b(remainderbits)).get());
        stream.readBitRepeatedly(false, remainderbits);
    }
    phyHeader->setChunkLength(stream.getPosition() - startPosition);
    return phyHeader;
}

} // namespace physicallayer
} // namespace inet

