//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/shortcut/ShortcutPhyHeaderSerializer.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/physicallayer/wireless/shortcut/ShortcutPhyHeader_m.h"

namespace inet {
namespace physicallayer {

Register_Serializer(ShortcutPhyHeader, ShortcutPhyHeaderSerializer);

void ShortcutPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    auto startPosition = stream.getLength();
    const auto& phyHeader = staticPtrCast<const ShortcutPhyHeader>(chunk);
    stream.writeUint16Be(phyHeader->getChunkLength().get<b>());
    stream.writeUint16Be(ProtocolGroup::getInetPhyProtocolGroup()->getProtocolNumber(phyHeader->getPayloadProtocol()));

    b remainders = phyHeader->getChunkLength() - (stream.getLength() - startPosition);
    if (remainders < b(0))
        throw cRuntimeError("ShortcutPhyHeader length = %d bit smaller than required %d bits", (int)phyHeader->getChunkLength().get<b>(), (int)(stream.getLength() - startPosition).get<b>());
    uint8_t remainderbits = remainders.get<b>() % 8;
    stream.writeByteRepeatedly('?', (remainders - b(remainderbits)).get<B>());
    stream.writeBitRepeatedly(false, remainderbits);
}

const Ptr<Chunk> ShortcutPhyHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto phyHeader = makeShared<ShortcutPhyHeader>();
    b headerLength = b(stream.readUint16Be());
    phyHeader->setChunkLength(headerLength);
    phyHeader->setPayloadProtocol(ProtocolGroup::getInetPhyProtocolGroup()->findProtocol(stream.readUint16Be()));

    b curLength = stream.getPosition() - startPosition;
    b remainders = headerLength - curLength;
    if (remainders < b(0)) {
        phyHeader->markIncorrect();
    }
    else {
        uint8_t remainderbits = remainders.get<b>() % 8;
        stream.readByteRepeatedly('?', (remainders - b(remainderbits)).get<B>());
        stream.readBitRepeatedly(false, remainderbits);
    }
    phyHeader->setChunkLength(stream.getPosition() - startPosition);
    return phyHeader;
}

} // namespace physicallayer
} // namespace inet

