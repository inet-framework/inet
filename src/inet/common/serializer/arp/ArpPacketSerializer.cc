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
#include "inet/common/serializer/arp/ArpPacketSerializer.h"
#include "inet/common/serializer/headers/ethernethdr.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"

namespace inet {

namespace serializer {

Register_Serializer(ArpPacket, ArpPacketSerializer);

MacAddress ArpPacketSerializer::readMACAddress(MemoryInputStream& stream, unsigned int size) const
{
    b curpos = stream.getPosition();
    MacAddress address = stream.readMACAddress();
    stream.seek(curpos + B(size));
    return address;
}

Ipv4Address ArpPacketSerializer::readIPv4Address(MemoryInputStream& stream, unsigned int size) const
{
    b curpos = stream.getPosition();
    Ipv4Address address = stream.readIPv4Address();
    stream.seek(curpos + B(size));
    return address;
}

void ArpPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& arpPacket = staticPtrCast<const ArpPacket>(chunk);
    stream.writeUint16Be(1); //ethernet
    stream.writeUint16Be(ETHERTYPE_IPv4);
    stream.writeByte(ETHER_ADDR_LEN);
    stream.writeByte(4);  // size of IPv4 address
    stream.writeUint16Be(arpPacket->getOpcode());
    stream.writeMACAddress(arpPacket->getSrcMacAddress());
    stream.writeIPv4Address(arpPacket->getSrcIpAddress());
    stream.writeMACAddress(arpPacket->getDestMacAddress());
    stream.writeIPv4Address(arpPacket->getDestIpAddress());
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
    arpPacket->setOpcode((ArpOpcode)stream.readUint16Be());   // arphdr->ar_op
    arpPacket->setSrcMacAddress(readMACAddress(stream, macAddressLength));
    arpPacket->setSrcIpAddress(readIPv4Address(stream, ipAddressLength));    // ar_spa
    arpPacket->setDestMacAddress(readMACAddress(stream, macAddressLength));
    arpPacket->setDestIpAddress(readIPv4Address(stream, ipAddressLength));   // ar_tpa
    return arpPacket;
}

} // namespace serializer

} // namespace inet

