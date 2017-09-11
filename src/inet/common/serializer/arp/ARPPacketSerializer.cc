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
#include "inet/common/serializer/headers/ethernethdr.h"
#include "inet/common/serializer/arp/ARPPacketSerializer.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"

namespace inet {

namespace serializer {

Register_Serializer(ARPPacket, ARPPacketSerializer);

MACAddress ARPPacketSerializer::readMACAddress(MemoryInputStream& stream, unsigned int size) const
{
    b curpos = stream.getPosition();
    MACAddress address = stream.readMACAddress();
    stream.seek(curpos + B(size));
    return address;
}

IPv4Address ARPPacketSerializer::readIPv4Address(MemoryInputStream& stream, unsigned int size) const
{
    b curpos = stream.getPosition();
    IPv4Address address = stream.readIPv4Address();
    stream.seek(curpos + B(size));
    return address;
}

void ARPPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& arpPacket = std::static_pointer_cast<const ARPPacket>(chunk);
    stream.writeUint16Be(1); //ethernet
    stream.writeUint16Be(ETHERTYPE_IPv4);
    stream.writeByte(ETHER_ADDR_LEN);
    stream.writeByte(4);  // size of IPv4 address
    stream.writeUint16Be(arpPacket->getOpcode());
    stream.writeMACAddress(arpPacket->getSrcMACAddress());
    stream.writeIPv4Address(arpPacket->getSrcIPAddress());
    stream.writeMACAddress(arpPacket->getDestMACAddress());
    stream.writeIPv4Address(arpPacket->getDestIPAddress());
}

const Ptr<Chunk> ARPPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    auto arpPacket = std::make_shared<ARPPacket>();
    if (stream.readUint16Be() != 1)
        arpPacket->markIncorrect();
    if (stream.readUint16Be() != ETHERTYPE_IPv4)
        arpPacket->markIncorrect();
    uint8_t macAddressLength = stream.readByte();     //ar_hln
    uint8_t ipAddressLength = stream.readByte();     //ar_pln
    arpPacket->setOpcode(stream.readUint16Be());   // arphdr->ar_op
    arpPacket->setSrcMACAddress(readMACAddress(stream, macAddressLength));
    arpPacket->setSrcIPAddress(readIPv4Address(stream, ipAddressLength));    // ar_spa
    arpPacket->setDestMACAddress(readMACAddress(stream, macAddressLength));
    arpPacket->setDestIPAddress(readIPv4Address(stream, ipAddressLength));   // ar_tpa
    return arpPacket;
}

} // namespace serializer

} // namespace inet

