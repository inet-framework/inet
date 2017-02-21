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
#include "inet/common/packet/SerializerRegistry.h"
#include "inet/common/serializer/application/ApplicationPacketSerializer.h"

namespace inet {

namespace serializer {

Register_Serializer(ApplicationPacket, ApplicationPacketSerializer);

void ApplicationPacketSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& applicationPacket = std::static_pointer_cast<const ApplicationPacket>(chunk);
    stream.writeUint8(applicationPacket->getSequenceNumber());
    // TODO: what if chunk size is set to something else
}

std::shared_ptr<Chunk> ApplicationPacketSerializer::deserialize(ByteInputStream& stream) const
{
    auto applicationPacket = std::make_shared<ApplicationPacket>();
    applicationPacket->setSequenceNumber(stream.readUint8());
    // TODO: what if chunk size is set to something else
    return applicationPacket;
}

} // namespace serializer

} // namespace inet

