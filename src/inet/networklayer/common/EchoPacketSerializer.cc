//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/common/EchoPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/common/EchoPacket_m.h"

namespace inet {

Register_Serializer(EchoPacket, EchoPacketSerializer);

void EchoPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& echoPacket = staticPtrCast<const EchoPacket>(chunk);
    stream.writeUint16Be(echoPacket->getType());
    stream.writeUint16Be(echoPacket->getIdentifier());
    stream.writeUint16Be(echoPacket->getSeqNumber());
}

const Ptr<Chunk> EchoPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto echoPacket = makeShared<EchoPacket>();
    echoPacket->setType(static_cast<inet::EchoProtocolType>(stream.readUint16Be()));
    echoPacket->setIdentifier(stream.readUint16Be());
    echoPacket->setSeqNumber(stream.readUint16Be());
    return echoPacket;
}

} // namespace inet

