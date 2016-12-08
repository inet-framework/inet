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
#include "inet/transportlayer/udp/UdpHeader.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

Register_Serializer(UdpHeader, IP_PROT, IP_PROT_UDP, UDPSerializer);

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
#if 0
    ASSERT(b.getPos() == 0);
    const FlatPacket *pkt = check_and_cast<const FlatPacket *>(_pkt);
    const UdpHeader *header = check_and_cast<const UdpHeader *>(pkt->peekHeader());
    int packetLength = header->getTotalLengthField();

    ASSERT(packetLength == pkt->getByteLength());       //FIXME error occured when serialize a corrupt packet

    b.writeUint16(header->getSourcePort());
    b.writeUint16(header->getDestinationPort());
    b.writeUint16(packetLength);
    unsigned int chksumPos = b.getPos();
    b.writeUint16(0);  // place for checksum

    int payloadLength = 0;
    for (int i=1; i<pkt->getNumChunks(); i++) {
        const FlatChunk *chunk = pkt->getChunk(i);
        SerializerBase::lookupAndSerialize(check_and_cast<const PacketChunk *>(chunk)->getPacket(), b, c, UNKNOWN, 0);
        payloadLength += chunk->getChunkByteLength();
    }
    b.fillNBytes(packetLength - UDP_HEADER_BYTES - payloadLength, 0);   // remained payload place

    unsigned int endPos = b.getPos();
    b.writeUint16To(chksumPos, TCPIPchecksum::checksum(IP_PROT_UDP, b._getBuf(), endPos, c.l3AddressesPtr, c.l3AddressesLength));
#endif
}

cPacket *UDPSerializer::deserialize(const Buffer &b, Context& c)
{
#if 0
    ASSERT(b.getPos() == 0);
    FlatPacket *pkt = new FlatPacket("parsed-udp");
    UdpHeader *header = new UdpHeader("parsed-udp");
    header->setSourcePort(b.readUint16());
    header->setDestinationPort(b.readUint16());
    unsigned int length = b.readUint16();
    header->setTotalLengthField(length);
    uint16_t chksum = b.readUint16();
    pkt->pushHeader(header);
    if (length > UDP_HEADER_BYTES) {
        unsigned int payloadLength = length - UDP_HEADER_BYTES;
        Buffer subBuffer(b, payloadLength);
        cPacket *encapPacket = serializers.byteArraySerializer.deserializePacket(subBuffer, c);
        b.accessNBytes(payloadLength);
        pkt->pushTrailer(new PacketChunk(encapPacket));
    }
    if (chksum != 0 && c.l3AddressesPtr && c.l3AddressesLength)
        chksum = TCPIPchecksum::checksum(IP_PROT_UDP, b._getBuf(), b.getPos(), c.l3AddressesPtr, c.l3AddressesLength);
    else chksum = 0;
    header->setIsChecksumCorrect(length >= UDP_HEADER_BYTES && chksum == 0 && pkt->getByteLength() == length);
    return pkt;
#endif
    return nullptr;
}

} // namespace serializer

} // namespace inet

