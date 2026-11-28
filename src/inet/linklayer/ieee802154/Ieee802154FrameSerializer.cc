// SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "inet/linklayer/ieee802154/Ieee802154FrameSerializer.h"
#include "inet/linklayer/ieee802154/Ieee802154FrameCodec.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {
Register_Serializer(Ieee802154Frame, Ieee802154FrameSerializer);

void Ieee802154FrameSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto frame = staticPtrCast<const Ieee802154Frame>(chunk);
    auto bytes = Ieee802154FrameCodec::encode(*frame);
    stream.writeBytes(bytes);
}

const Ptr<Chunk> Ieee802154FrameSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    std::vector<uint8_t> bytes;
    stream.readBytes(bytes, B(stream.getRemainingLength()));
    return Ieee802154FrameCodec::decode(bytes);
}
}
