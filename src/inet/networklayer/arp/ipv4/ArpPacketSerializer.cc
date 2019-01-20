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
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/arp/ipv4/ArpPacketSerializer.h"

namespace inet {

Register_Serializer(ArpPacket, ArpPacketSerializer);

MacAddress ArpPacketSerializer::readMacAddress(MemoryInputStream& stream, unsigned int size) const
{
    b curpos = stream.getPosition();
    MacAddress address = stream.readMacAddress();
    stream.seek(curpos + B(size));
    return address;
}

Ipv4Address ArpPacketSerializer::readIpv4Address(MemoryInputStream& stream, unsigned int size) const
{
    b curpos = stream.getPosition();
    Ipv4Address address = stream.readIpv4Address();
    stream.seek(curpos + B(size));
    return address;
}

void ArpPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& arpPacket = staticPtrCast<const ArpPacket>(chunk);
    stream.writeUint16Be(1); //ethernet
    stream.writeUint16Be(ETHERTYPE_IPv4);
    stream.writeByte(MAC_ADDRESS_SIZE);
    stream.writeByte(4);  // size of IPv4 address
    stream.writeUint16Be(arpPacket->getOpcode());
    stream.writeMacAddress(arpPacket->getSrcMacAddress());
    stream.writeIpv4Address(arpPacket->getSrcIpAddress());
    stream.writeMacAddress(arpPacket->getDestMacAddress());
    stream.writeIpv4Address(arpPacket->getDestIpAddress());
}

const Ptr<Chunk> ArpPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto arpPacket = makeShared<ArpPacket>();
    if (stream.readUint16Be() != 1)
        arpPacket->markIncorrect();
    if (stream.readUint16Be() != ETHERTYPE_IPv4)
        arpPacket->markIncorrect();
    uint8_t macAddressLength = stream.readByte();     //ar_hln
    uint8_t ipAddressLength = stream.readByte();     //ar_pln
    arpPacket->setOpcode(static_cast<ArpOpcode>(stream.readUint16Be()));   // arphdr->ar_op
    arpPacket->setSrcMacAddress(readMacAddress(stream, macAddressLength));
    arpPacket->setSrcIpAddress(readIpv4Address(stream, ipAddressLength));    // ar_spa
    arpPacket->setDestMacAddress(readMacAddress(stream, macAddressLength));
    arpPacket->setDestIpAddress(readIpv4Address(stream, ipAddressLength));   // ar_tpa
    return arpPacket;
}

} // namespace inet

