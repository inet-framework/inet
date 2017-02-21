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

#include "inet/common/packet/SerializerRegistry.h"
#include "inet/common/serializer/headers/ethernethdr.h"
#include "inet/common/serializer/headerserializers/arp/ARPPacketSerializer.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"

namespace inet {

namespace serializer {

Register_Serializer(ARPPacket, ARPPacketSerializer);

/*
struct arphdr {
        uint16_t ar_hrd;                 // Hardware type (16 bits)
        uint16_t ar_pro;                 // Protocol type (16 bits)
        uint8_t ar_hln;                  // Byte length of each hardware address (n) (8 bits)
        uint8_t ar_pln;                  // Byte length of each protocol address (m) (8 bits)
        uint16_t ar_op;                  // Operation code (16 bits)
        uint8_t ar_sha[ETHER_ADDR_LEN];  // source hardware address (n bytes)
        uint32_t ar_spa;                 // source protocol address (m bytes)
        uint8_t ar_tha[ETHER_ADDR_LEN];  // target hardware address (n bytes)
        uint32_t ar_tpa;                 // target protocol address (m bytes)
} __PACKED__;
*/

MACAddress ARPPacketSerializer::readMACAddress(ByteInputStream& stream, unsigned int size) const
{
    unsigned int curpos = stream.getPosition();
    MACAddress address = stream.readMACAddress();
    stream.seek(curpos + size);
    return address;
}

IPv4Address ARPPacketSerializer::readIPv4Address(ByteInputStream& stream, unsigned int size) const
{
    unsigned int curpos = stream.getPosition();
    IPv4Address address = stream.readIPv4Address();
    stream.seek(curpos + size);
    return address;
}

void ARPPacketSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    const auto& arpPacket = std::static_pointer_cast<const ARPPacket>(chunk);
    stream.writeUint16(1); //ethernet
    stream.writeUint16(ETHERTYPE_IPv4);
    stream.writeByte(ETHER_ADDR_LEN);
    stream.writeByte(4);  // size of IPv4 address
    stream.writeUint16(arpPacket->getOpcode());
    stream.writeMACAddress(arpPacket->getSrcMACAddress());
    stream.writeIPv4Address(arpPacket->getSrcIPAddress());
    stream.writeMACAddress(arpPacket->getDestMACAddress());
    stream.writeIPv4Address(arpPacket->getDestIPAddress());
}

std::shared_ptr<Chunk> ARPPacketSerializer::deserialize(ByteInputStream& stream) const
{
    auto arpPacket = std::make_shared<ARPPacket>();
    if (stream.readUint16() != 1)
        arpPacket->markIncorrect();
    if (stream.readUint16() != ETHERTYPE_IPv4)
        arpPacket->markIncorrect();
    uint8_t macAddressLength = stream.readByte();     //ar_hln
    uint8_t ipAddressLength = stream.readByte();     //ar_pln
    arpPacket->setOpcode(stream.readUint16());   // arphdr->ar_op
    arpPacket->setSrcMACAddress(readMACAddress(stream, macAddressLength));
    arpPacket->setSrcIPAddress(readIPv4Address(stream, ipAddressLength));    // ar_spa
    arpPacket->setDestMACAddress(readMACAddress(stream, macAddressLength));
    arpPacket->setDestIPAddress(readIPv4Address(stream, ipAddressLength));   // ar_tpa
    // TODO: arpPacket->setChunkLength(header + m * ... n * ...);
    return arpPacket;
}

} // namespace serializer

} // namespace inet

