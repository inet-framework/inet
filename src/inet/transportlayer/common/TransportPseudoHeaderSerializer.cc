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
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/common/TransportPseudoHeaderSerializer.h"

namespace inet {

namespace serializer {

Register_Serializer(TransportPseudoHeader, TransportPseudoHeaderSerializer);

void TransportPseudoHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& transportPseudoHeader = std::static_pointer_cast<const TransportPseudoHeader>(chunk);
    stream.writeIPv4Address(transportPseudoHeader->getSrcAddress().toIPv4());
    stream.writeIPv4Address(transportPseudoHeader->getDestAddress().toIPv4());
    stream.writeByte(0);
    stream.writeByte(transportPseudoHeader->getProtocolId());
    stream.writeUint16(transportPseudoHeader->getPacketLength());
}

std::shared_ptr<Chunk> TransportPseudoHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto transportPseudoHeader = std::make_shared<TransportPseudoHeader>();
    transportPseudoHeader->setSrcAddress(stream.readIPv4Address());
    transportPseudoHeader->setDestAddress(stream.readIPv4Address());
    stream.readByte();
    transportPseudoHeader->setProtocolId(stream.readByte());
    transportPseudoHeader->setPacketLength(stream.readUint16());
    return transportPseudoHeader;
}

} // namespace serializer

} // namespace inet

