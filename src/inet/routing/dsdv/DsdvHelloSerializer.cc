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
#include "inet/routing/dsdv/DsdvHello_m.h"
#include "inet/routing/dsdv/DsdvHelloSerializer.h"

namespace inet {

Register_Serializer(DsdvHello, DsdvHelloSerializer);

void DsdvHelloSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
	const auto& dsdvHello = staticPtrCast<const DsdvHello>(chunk);
	stream.writeUint32Be(dsdvHello->getSrcAddress().getInt());
	stream.writeUint32Be(dsdvHello->getSequencenumber());
	stream.writeUint32Be(dsdvHello->getNextAddress().getInt());
	stream.writeUint32Be(dsdvHello->getHopdistance());
}

const Ptr<Chunk> DsdvHelloSerializer::deserialize(MemoryInputStream& stream) const
{
	auto dsdvHello = makeShared<DsdvHello>();
	dsdvHello->setSrcAddress(Ipv4Address(stream.readUint32Be()));
	dsdvHello->setSequencenumber(stream.readUint32Be());
	dsdvHello->setNextAddress(Ipv4Address(stream.readUint32Be()));
	dsdvHello->setHopdistance(stream.readUint32Be());
	dsdvHello->setChunkLength(b(128));
	return dsdvHello;
}

} // namespace inet

