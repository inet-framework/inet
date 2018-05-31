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
#include "inet/routing/rip/RipPacket_m.h"
#include "inet/routing/rip/RipDataSerializer.h"

namespace inet {

Register_Serializer(RipPacket, RipDataSerializer);

void RipDataSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ripPacket = staticPtrCast<const RipPacket>(chunk);

    stream.writeUint8(ripPacket->getCommand());
    stream.writeUint8(2); // RIP version

    int numEntries = ripPacket->getEntryArraySize();
    stream.writeUint16Be(numEntries);

    // iterate over each entry and write to stream
    for (int i = 0; i < numEntries; ++i) {
        const RipEntry& entry = ripPacket->getEntry(i);
        stream.writeUint16Be(entry.addressFamilyId);
        stream.writeUint16Be(entry.routeTag);
        stream.writeIpv4Address(entry.address.toIpv4());
        stream.writeUint32Be(entry.prefixLength);
        stream.writeIpv4Address(entry.nextHop.toIpv4());
        stream.writeUint32Be(entry.metric);
    }
}

const Ptr<Chunk> RipDataSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ripPacket = makeShared<RipPacket>();

    ripPacket->setCommand((inet::RipCommand)stream.readUint8());
    int ripVer = stream.readUint8();
    ASSERT(ripVer == 2);

    int numEntries = stream.readUint16Be();
    ripPacket->setEntryArraySize(numEntries);

    for (int i = 0; i < numEntries; ++i) {

        RipEntry entry = {};

        entry.addressFamilyId = (inet::RipAf)stream.readUint16Be();
        entry.routeTag = stream.readUint16Be();
        entry.address = stream.readIpv4Address();
        entry.prefixLength = stream.readUint32Be();
        entry.nextHop = stream.readIpv4Address();
        entry.metric = stream.readUint32Be();

        ripPacket->setEntry(i, entry);
    }

    return ripPacket;
}

} // namespace inet

