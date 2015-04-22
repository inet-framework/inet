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

#include "inet/common/serializer/SerializerUtil.h"

#include "inet/common/serializer/headerserializers/arp/ARPSerializer.h"

#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/ethernethdr.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"

namespace inet {
namespace serializer {

Register_Serializer(ARPPacket, ETHERTYPE, ETHERTYPE_ARP, ARPSerializer);

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

MACAddress ARPSerializer::readMACAddress(const Buffer& b, unsigned int size)
{
    unsigned int curpos = b.getPos();
    MACAddress addr = b.readMACAddress();
    b.seek(curpos + size);
    return addr;
}

IPv4Address ARPSerializer::readIPv4Address(const Buffer& b, unsigned int size)
{
    unsigned int curpos = b.getPos();
    IPv4Address addr = b.readIPv4Address();
    b.seek(curpos + size);
    return addr;
}

void ARPSerializer::serialize(const cPacket *_pkt, Buffer &b, Context& context)
{
    const ARPPacket *pkt = check_and_cast<const ARPPacket *>(_pkt);
    b.writeUint16(1); //ethernet
    b.writeUint16(ETHERTYPE_IPv4);
    b.writeByte(ETHER_ADDR_LEN);
    b.writeByte(4);  // size of IPv4 address
    b.writeUint16(pkt->getOpcode());
    b.writeMACAddress(pkt->getSrcMACAddress());
    b.writeIPv4Address(pkt->getSrcIPAddress());
    b.writeMACAddress(pkt->getDestMACAddress());
    b.writeIPv4Address(pkt->getDestIPAddress());
    if (pkt->getEncapsulatedPacket())
        throw cRuntimeError("ARPSerializer: encapsulated packet not supported!");
}

cPacket* ARPSerializer::deserialize(const Buffer &b, Context& context)
{
    ARPPacket *pkt = new ARPPacket("parsed ARP");

    uint16_t x;
    x = b.readUint16();   // 1
    if (x != 1)
        pkt->setBitError(true);
    x = b.readUint16();   // ETHERTYPE_IP
    if (x != ETHERTYPE_IPv4)
        pkt->setBitError(true);
    uint8_t n = b.readByte();     //ar_hln
    uint8_t m = b.readByte();     //ar_pln
    pkt->setOpcode(b.readUint16());   // arphdr->ar_op
    MACAddress temp;
    pkt->setSrcMACAddress(readMACAddress(b, n));
    pkt->setSrcIPAddress(readIPv4Address(b, m));    // ar_spa
    pkt->setDestMACAddress(readMACAddress(b, n));
    pkt->setDestIPAddress(readIPv4Address(b, m));   // ar_tpa
    pkt->setByteLength(b.getPos());
    return pkt;
}

} // namespace serializer

} // namespace inet

