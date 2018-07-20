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

#include "inet/common/Protocol.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/common/TransportPseudoHeaderSerializer.h"

namespace inet {

Register_Serializer(TransportPseudoHeader, TransportPseudoHeaderSerializer);

void TransportPseudoHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    //FIXME: ipv6, generic ????
    const auto& transportPseudoHeader = staticPtrCast<const TransportPseudoHeader>(chunk);
    auto nwProtId = transportPseudoHeader->getNetworkProtocolId();
    if (nwProtId == Protocol::ipv4.getId()) {
        ASSERT(transportPseudoHeader->getChunkLength() == B(12));
        stream.writeIpv4Address(transportPseudoHeader->getSrcAddress().toIpv4());
        stream.writeIpv4Address(transportPseudoHeader->getDestAddress().toIpv4());
        stream.writeByte(0);
        stream.writeByte(transportPseudoHeader->getProtocolId());
        stream.writeUint16Be(B(transportPseudoHeader->getPacketLength()).get());
    }
    else
    if (nwProtId == Protocol::ipv6.getId()) {
        ASSERT(transportPseudoHeader->getChunkLength() == B(40));
        stream.writeIpv6Address(transportPseudoHeader->getSrcAddress().toIpv6());
        stream.writeIpv6Address(transportPseudoHeader->getDestAddress().toIpv6());
        stream.writeUint32Be(B(transportPseudoHeader->getPacketLength()).get());
        stream.writeByte(0);
        stream.writeByte(0);
        stream.writeByte(0);
        stream.writeByte(transportPseudoHeader->getProtocolId());
    }
    else
        throw cRuntimeError("Unknown network protocol: %d", nwProtId);
}

const Ptr<Chunk> TransportPseudoHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    throw cRuntimeError("TransportPseudoHeader is not a valid deserializable data");
}

} // namespace inet

