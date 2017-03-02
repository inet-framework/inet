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

#include "inet/common/packet/serializer/SerializerRegistry.h"
#include "inet/common/serializer/udp/UdpHeaderSerializer.h"
#include "inet/transportlayer/udp/UdpHeader.h"

namespace inet {

namespace serializer {

Register_Serializer(UdpHeader, UdpHeaderSerializer);

void UdpHeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& udpHeader = std::static_pointer_cast<const UdpHeader>(chunk);
    stream.writeUint16(udpHeader->getSourcePort());
    stream.writeUint16(udpHeader->getDestinationPort());
    stream.writeUint16(udpHeader->getTotalLengthField());
    if (udpHeader->getCrcMode() != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize UDP header without a properly computed CRC");
    stream.writeUint16(udpHeader->getCrc());
}

std::shared_ptr<Chunk> UdpHeaderSerializer::deserialize(ByteInputStream& stream) const
{
    auto udpHeader = std::make_shared<UdpHeader>();
    udpHeader->setSourcePort(stream.readUint16());
    udpHeader->setDestinationPort(stream.readUint16());
    udpHeader->setTotalLengthField(stream.readUint16());
    udpHeader->setCrc(stream.readUint16());
    udpHeader->setCrcMode(CRC_COMPUTED);
    return udpHeader;
}

} // namespace serializer

} // namespace inet

