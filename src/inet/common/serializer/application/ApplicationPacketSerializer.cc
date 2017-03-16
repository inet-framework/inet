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

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/serializer/application/ApplicationPacketSerializer.h"

namespace inet {

namespace serializer {

Register_Serializer(ApplicationPacket, ApplicationPacketSerializer);

void ApplicationPacketSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& applicationPacket = std::static_pointer_cast<const ApplicationPacket>(chunk);
    stream.writeUint32(byte(applicationPacket->getChunkLength()).get());
    stream.writeUint32(applicationPacket->getSequenceNumber());
    stream.writeByteRepeatedly('?', byte(applicationPacket->getChunkLength() - byte(8)).get());
}

std::shared_ptr<Chunk> ApplicationPacketSerializer::deserialize(ByteInputStream& stream) const
{
    auto applicationPacket = std::make_shared<ApplicationPacket>();
    byte chunkLength = byte(stream.readUint32());
    applicationPacket->setChunkLength(chunkLength);
    applicationPacket->setSequenceNumber(stream.readUint32());
    stream.readByteRepeatedly('?', byte(chunkLength - byte(8)).get());
    return applicationPacket;
}

} // namespace serializer

} // namespace inet

