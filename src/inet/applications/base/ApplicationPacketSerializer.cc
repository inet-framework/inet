//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/base/ApplicationPacketSerializer.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(ApplicationPacket, ApplicationPacketSerializer);

void ApplicationPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& applicationPacket = staticPtrCast<const ApplicationPacket>(chunk);
    stream.writeUint32Be(B(applicationPacket->getChunkLength()).get());
    stream.writeUint32Be(applicationPacket->getSequenceNumber());
    int64_t remainders = B(applicationPacket->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("ApplicationPacket length = %d smaller than required %d bytes", (int)B(applicationPacket->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    stream.writeByteRepeatedly('?', remainders);
}

const Ptr<Chunk> ApplicationPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto applicationPacket = makeShared<ApplicationPacket>();
    B dataLength = B(stream.readUint32Be());
    applicationPacket->setSequenceNumber(stream.readUint32Be());
    B remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= B(0));
    stream.readByteRepeatedly('?', B(remainders).get());
    return applicationPacket;
}

} // namespace inet

