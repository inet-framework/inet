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

void GenericAppMsgSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    int64_t startPos = stream.getPosition();
    const auto& msg = std::static_pointer_cast<const GenericAppMsg>(chunk);
    stream.writeUint32(byte(msg->getChunkLength()).get());
    stream.writeUint32(msg->getExpectedReplyLength());
    stream.writeUint64(SimTime(msg->getReplyDelay()).raw());
    stream.writeByte(msg->getServerClose());
    int64_t remainders = byte(msg->getChunkLength()).get() - (stream.getPosition() - startPos);
    if (remainders < 0)
        throw cRuntimeError("GenericAppMsg length = %d smaller than required %d bytes", (int)byte(msg->getChunkLength()).get(), (int)(stream.getPosition() - startPos));
    stream.writeByteRepeatedly('?', remainders);
}

std::shared_ptr<Chunk> GenericAppMsgSerializer::deserialize(ByteInputStream& stream) const
{
    int64_t startPos = stream.getPosition();
    auto msg = std::make_shared<GenericAppMsg>();
    byte chunkLength = byte(stream.readUint32());
    msg->setChunkLength(chunkLength);
    msg->setExpectedReplyLength(stream.readUint32());
    int64_t delayraw = stream.readUint64();
    msg->setReplyDelay(SimTime(delayraw).dbl());
    msg->setServerClose(stream.readByte() ? true : false);
    int64_t remainders = byte(msg->getChunkLength()).get() - (stream.getPosition() - startPos);
    ASSERT(remainders >= 0);
    stream.readByteRepeatedly('?', remainders);
    return msg;
}

} // namespace serializer

} // namespace inet

