//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/applications/tcpapp/GenericAppMsgSerializer.h"
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

