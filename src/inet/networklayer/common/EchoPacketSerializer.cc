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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/common/EchoPacket_m.h"
#include "inet/networklayer/common/EchoPacketSerializer.h"

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
