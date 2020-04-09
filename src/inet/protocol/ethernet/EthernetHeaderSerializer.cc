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
#include "inet/protocol/ethernet/EthernetHeaders_m.h"
#include "inet/protocol/ethernet/EthernetHeaderSerializer.h"

namespace inet {

Register_Serializer(Ieee8023MacAddresses, Ieee8023MacAddressesSerializer);
Register_Serializer(Ieee8023TypeOrLength, Ieee8023TypeOrLengthSerializer);

void Ieee8023MacAddressesSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8023MacAddresses>(chunk);
    stream.writeMacAddress(header->getDest());
    stream.writeMacAddress(header->getSrc());
}

const Ptr<Chunk> Ieee8023MacAddressesSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<Ieee8023MacAddresses>();
    header->setDest(stream.readMacAddress());
    header->setSrc(stream.readMacAddress());
    return header;
}

void Ieee8023TypeOrLengthSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee8023TypeOrLength>(chunk);
    stream.writeUint16Be(header->getTypeOrLength());
}

const Ptr<Chunk> Ieee8023TypeOrLengthSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<Ieee8023TypeOrLength>();
    header->setTypeOrLength(stream.readUint16Be());
    return header;
}

} // namespace inet

