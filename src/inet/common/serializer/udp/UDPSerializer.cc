//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/serializer/SerializerUtil.h"

#include "inet/common/serializer/udp/UDPSerializer.h"

#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/udp/headers/udphdr.h"

#include "inet/common/RawPacket.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/transportlayer/udp/UDPPacket.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

Register_Serializer(UDPPacket, IP_PROT, IP_PROT_UDP, UDPSerializer);

/*
 * Udp protocol header.
 * Per RFC 768, September, 1981.
 */
#if 0
struct udphdr
{
    u_short uh_sport;    /* source port */
    u_short uh_dport;    /* destination port */
    u_short uh_ulen;    /* udp packet length (udp header + payload) */
    u_short uh_sum;    /* udp checksum */
};
#endif

void UDPSerializer::serialize(const cPacket *_pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);
    const UDPPacket *pkt = check_and_cast<const UDPPacket *>(_pkt);
    int packetLength = pkt->getByteLength();
    b.writeUint16(pkt->getSourcePort());
    b.writeUint16(pkt->getDestinationPort());
    b.writeUint16(pkt->getTotalLengthField());
    unsigned int chksumPos = b.getPos();
    b.writeUint16(0);  // place for checksum
    const cPacket *encapPkt = pkt->getEncapsulatedPacket();
    if (encapPkt) {
        SerializerBase::lookupAndSerialize(encapPkt, b, c, UNKNOWN, 0);
    }
    else {
        b.fillNBytes(packetLength - UDP_HEADER_BYTES, 0);   // payload place
    }
    unsigned int endPos = b.getPos();
    b.writeUint16To(chksumPos, TCPIPchecksum::checksum(IP_PROT_UDP, b._getBuf(), endPos, c.l3AddressesPtr, c.l3AddressesLength));
}

cPacket *UDPSerializer::deserialize(const Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);
    UDPPacket *pkt = new UDPPacket("parsed-udp");
    pkt->setSourcePort(b.readUint16());
    pkt->setDestinationPort(b.readUint16());
    unsigned int length = b.readUint16();
    pkt->setTotalLengthField(length);
    uint16_t chksum = b.readUint16();
    if (length > UDP_HEADER_BYTES) {
        unsigned int payloadLength = length - UDP_HEADER_BYTES;
        Buffer subBuffer(b, payloadLength);
        cPacket *encapPacket = serializers.byteArraySerializer.deserializePacket(subBuffer, c);
        b.accessNBytes(payloadLength);
        pkt->encapsulate(encapPacket);
    }
    if (chksum != 0 && c.l3AddressesPtr && c.l3AddressesLength)
        chksum = TCPIPchecksum::checksum(IP_PROT_UDP, b._getBuf(), b.getPos(), c.l3AddressesPtr, c.l3AddressesLength);
    else chksum = 0;
    if (length < UDP_HEADER_BYTES || chksum != 0 || pkt->getByteLength() != length)
        pkt->setBitError(true);
    return pkt;
}

} // namespace serializer

} // namespace inet

