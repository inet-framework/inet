//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/GenericAppMsgSerializer.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(GenericAppMsg, GenericAppMsgSerializer);

void GenericAppMsgSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& msg = staticPtrCast<const GenericAppMsg>(chunk);
    stream.writeUint32Be(B(msg->getChunkLength()).get());
    stream.writeUint32Be(B(msg->getExpectedReplyLength()).get());
    stream.writeUint64Be(SimTime(msg->getReplyDelay()).raw());
    stream.writeByte(msg->getServerClose());
    int64_t remainders = B(msg->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("GenericAppMsg length = %d smaller than required %d bytes", (int)B(msg->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    stream.writeByteRepeatedly('?', remainders);
}

const Ptr<Chunk> GenericAppMsgSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto msg = makeShared<GenericAppMsg>();
    B dataLength = B(stream.readUint32Be());
    msg->setExpectedReplyLength(B(stream.readUint32Be()));
    int64_t delayraw = stream.readUint64Be();
    msg->setReplyDelay(SimTime(delayraw).dbl());
    msg->setServerClose(stream.readByte() ? true : false);
    B remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= B(0));
    stream.readByteRepeatedly('?', remainders.get());
    return msg;
}

} // namespace inet

