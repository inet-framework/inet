//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/acking/AckingMacHeaderSerializer.h"

namespace inet {

Register_Serializer(AckingMacHeader, AckingMacHeaderSerializer);

void AckingMacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto macHeader = staticPtrCast<const AckingMacHeader>(chunk);
    stream.writeMacAddress(macHeader->getSrc());
    stream.writeMacAddress(macHeader->getDest());
    stream.writeUint16Be(macHeader->getNetworkProtocol());
}

const Ptr<Chunk> AckingMacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto macHeader = makeShared<AckingMacHeader>();
    macHeader->setSrc(stream.readMacAddress());
    macHeader->setDest(stream.readMacAddress());
    macHeader->setNetworkProtocol(stream.readUint16Be());
    return macHeader;
}

} // namespace inet

