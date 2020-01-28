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
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/transportlayer/udp/UdpHeaderSerializer.h"

namespace inet {

Register_Serializer(UdpHeader, UdpHeaderSerializer);

void UdpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& udpHeader = staticPtrCast<const UdpHeader>(chunk);
    stream.writeUint16Be(udpHeader->getSourcePort());
    stream.writeUint16Be(udpHeader->getDestinationPort());
    stream.writeUint16Be(B(udpHeader->getTotalLengthField()).get());
    auto crcMode = udpHeader->getCrcMode();
    if (crcMode != CRC_DISABLED && crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize UDP header without turned off or properly computed CRC, try changing the value of crcMode parameter for Udp");
    stream.writeUint16Be(udpHeader->getCrc());
}

const Ptr<Chunk> UdpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto udpHeader = makeShared<UdpHeader>();
    udpHeader->setSourcePort(stream.readUint16Be());
    udpHeader->setDestinationPort(stream.readUint16Be());
    udpHeader->setTotalLengthField(B(stream.readUint16Be()));
    auto crc = stream.readUint16Be();
    udpHeader->setCrc(crc);
    udpHeader->setCrcMode(crc == 0 ? CRC_DISABLED : CRC_COMPUTED);
    return udpHeader;
}

} // namespace inet

