//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/shortcut/ShortcutMacHeaderSerializer.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/shortcut/ShortcutMacHeader_m.h"

namespace inet {
namespace physicallayer {

Register_Serializer(ShortcutMacHeader, ShortcutMacHeaderSerializer);

void ShortcutMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    auto startPosition = stream.getLength();
    const auto& header = staticPtrCast<const ShortcutMacHeader>(chunk);
    stream.writeUint16Be(header->getChunkLength().get<b>());
    stream.writeUint16Be(ProtocolGroup::getEthertypeProtocolGroup()->getProtocolNumber(header->getPayloadProtocol()));

    b remainders = header->getChunkLength() - (stream.getLength() - startPosition);
    if (remainders < b(0))
        throw cRuntimeError("ShortcutMacHeader length = %d bit smaller than required %d bits", (int)header->getChunkLength().get<b>(), (int)(stream.getLength() - startPosition).get<b>());
    uint8_t remainderbits = remainders.get<b>() % 8;
    stream.writeByteRepeatedly('?', (remainders - b(remainderbits)).get<B>());
    stream.writeBitRepeatedly(false, remainderbits);
}

const Ptr<Chunk> ShortcutMacHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto phyHeader = makeShared<ShortcutMacHeader>();
    b headerLength = b(stream.readUint16Be());
    phyHeader->setChunkLength(headerLength);
    phyHeader->setPayloadProtocol(ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(stream.readUint16Be()));

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

