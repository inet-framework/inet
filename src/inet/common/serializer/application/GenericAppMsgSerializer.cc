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
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/serializer/application/GenericAppMsgSerializer.h"

namespace inet {

namespace serializer {

Register_Serializer(GenericAppMsg, GenericAppMsgSerializer);

void GenericAppMsgSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    const auto& msg = std::static_pointer_cast<const GenericAppMsg>(chunk);
    stream.writeUint32Be(byte(msg->getChunkLength()).get());
    stream.writeUint32Be(msg->getExpectedReplyLength());
    stream.writeUint64Be(SimTime(msg->getReplyDelay()).raw());
    stream.writeByte(msg->getServerClose());
    int64_t remainders = byte(msg->getChunkLength() - (stream.getLength() - startPosition)).get();
    if (remainders < 0)
        throw cRuntimeError("GenericAppMsg length = %d smaller than required %d bytes", (int)byte(msg->getChunkLength()).get(), (int)byte(stream.getLength() - startPosition).get());
    stream.writeByteRepeatedly('?', remainders);
}

Ptr<Chunk> GenericAppMsgSerializer::deserialize(MemoryInputStream& stream) const
{
    auto startPosition = stream.getPosition();
    auto msg = std::make_shared<GenericAppMsg>();
    byte dataLength = byte(stream.readUint32Be());
    msg->setExpectedReplyLength(stream.readUint32Be());
    int64_t delayraw = stream.readUint64Be();
    msg->setReplyDelay(SimTime(delayraw).dbl());
    msg->setServerClose(stream.readByte() ? true : false);
    byte remainders = dataLength - (stream.getPosition() - startPosition);
    ASSERT(remainders >= byte(0));
    stream.readByteRepeatedly('?', byte(remainders).get());
    return msg;
}

} // namespace serializer

} // namespace inet

