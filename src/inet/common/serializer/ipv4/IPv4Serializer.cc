//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2010 Zoltan Bojthe
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

#include <algorithm>    // std::min
#include <typeinfo>

#include "inet/common/serializer/SerializerUtil.h"

#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/defs.h"

#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/ipv4/headers/ip.h"
#include "inet/common/serializer/ipv4/ICMPSerializer.h"
#include "inet/common/serializer/ipv4/IGMPSerializer.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

#if defined(_MSC_VER)
#undef s_addr    /* MSVC #definition interferes with us */
#endif // if defined(_MSC_VER)

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr

namespace inet {

namespace serializer {

Register_Serializer(IPv4Datagram, ETHERTYPE, ETHERTYPE_IPv4, IPv4Serializer);

void IPv4Serializer::serialize(const cPacket *pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    if (typeid(*pkt) != typeid(IPv4Datagram)) {
        if (c.throwOnSerializerNotFound)
            throw cRuntimeError("IPv4Serializer: class '%s' not accepted", pkt->getClassName());
        EV_ERROR << "IPv4Serializer: class '" << pkt->getClassName() << "' not accepted.\n";
        b.fillNBytes(pkt->getByteLength(), '?');
        return;
    }

    struct ip *ip = (struct ip *)b.accessNBytes(IP_HEADER_BYTES);
    if (!ip) {
        EV_ERROR << "IPv4Serializer: not enough space for IPv4 header.\n";
        return;
    }
    const IPv4Datagram *dgram = check_and_cast<const IPv4Datagram *>(pkt);
    ASSERT((dgram->getHeaderLength() & 3) == 0);
    ip->ip_hl = dgram->getHeaderLength() >> 2;
    ip->ip_v = dgram->getVersion();
    ip->ip_tos = dgram->getTypeOfService();
    ip->ip_id = htons(dgram->getIdentification());
    ASSERT((dgram->getFragmentOffset() & 7) == 0);
    uint16_t ip_off = dgram->getFragmentOffset() / 8;
    if (dgram->getMoreFragments())
        ip_off |= IP_MF;
    if (dgram->getDontFragment())
        ip_off |= IP_DF;
    ip->ip_off = htons(ip_off);
    ip->ip_ttl = dgram->getTimeToLive();
    ip->ip_p = dgram->getTransportProtocol();
    ip->ip_src.s_addr = htonl(dgram->getSrcAddress().getInt());
    ip->ip_dst.s_addr = htonl(dgram->getDestAddress().getInt());
    ip->ip_len = htons(dgram->getTotalLengthField());
    ip->ip_sum = 0;
    c.l3AddressesPtr = &ip->ip_src.s_addr;
    c.l3AddressesLength = sizeof(ip->ip_src.s_addr) + sizeof(ip->ip_dst.s_addr);

    if (dgram->getHeaderLength() > IP_HEADER_BYTES) {
        EV_ERROR << "Serializing an IPv4 packet with options. Dropping the options.\n";
        b.fillNBytes(dgram->getHeaderLength() - IP_HEADER_BYTES, 0);
    }

    const cPacket *encapPacket = dgram->getEncapsulatedPacket();
    unsigned int payloadLength = dgram->getByteLength() - b.getPos();

    if (encapPacket) {
        unsigned int totalLength = encapPacket->getByteLength();
        int fragmentOffset = dgram->getFragmentOffset();
        if ((dgram->getMoreFragments() || fragmentOffset != 0) && (payloadLength < totalLength)) {  // IP fragment  //FIXME hack: encapsulated packet contains entire packet if payloadLength < totalLength
            char *buf = new char[totalLength];
            Buffer tmpBuffer(buf, totalLength);
            SerializerBase::lookupAndSerialize(encapPacket, tmpBuffer, c, IP_PROT, dgram->getTransportProtocol());
            tmpBuffer.seek(fragmentOffset);
            b.writeNBytes(tmpBuffer, payloadLength);
            delete [] buf;
        }
        else    // no fragmentation, or the encapsulated packet is represents only the fragment
            SerializerBase::lookupAndSerialize(encapPacket, b, c, IP_PROT, dgram->getTransportProtocol());
    }
    else {
        b.fillNBytes(payloadLength, '?');
    }

    ip->ip_sum = htons(TCPIPchecksum::checksum(ip, IP_HEADER_BYTES));
}

cPacket* IPv4Serializer::deserialize(const Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    IPv4Datagram *dest = new IPv4Datagram("parsed-ipv4");
    unsigned int bufsize = b.getRemainingSize();
    const struct ip *ip = static_cast<const struct ip *>(b.accessNBytes(IP_HEADER_BYTES));
    if (!ip ) {
        delete dest;
        return nullptr;
    }
    unsigned int totalLength, headerLength;
    c.l3AddressesPtr = &ip->ip_src.s_addr;
    c.l3AddressesLength = sizeof(ip->ip_src.s_addr) + sizeof(ip->ip_dst.s_addr);

    dest->setVersion(ip->ip_v);
    dest->setHeaderLength(IP_HEADER_BYTES);
    dest->setSrcAddress(IPv4Address(ntohl(ip->ip_src.s_addr)));
    dest->setDestAddress(IPv4Address(ntohl(ip->ip_dst.s_addr)));
    dest->setTransportProtocol(ip->ip_p);
    dest->setTimeToLive(ip->ip_ttl);
    dest->setIdentification(ntohs(ip->ip_id));
    uint16_t ip_off = ntohs(ip->ip_off);
    dest->setMoreFragments((ip_off & IP_MF) != 0);
    dest->setDontFragment((ip_off & IP_DF) != 0);
    dest->setFragmentOffset((ntohs(ip->ip_off) & IP_OFFMASK) * 8);
    dest->setTypeOfService(ip->ip_tos);
    totalLength = ntohs(ip->ip_len);
    dest->setTotalLengthField(totalLength);
    headerLength = ip->ip_hl << 2;

    if (headerLength < IP_HEADER_BYTES) {
        dest->setBitError(true);
        headerLength = IP_HEADER_BYTES;
    }
    if (headerLength > b._getBufSize() || TCPIPchecksum::checksum(ip, headerLength) != 0)
        dest->setBitError(true);

    if (headerLength > IP_HEADER_BYTES) {
        EV_ERROR << "Handling a captured IPv4 packet with options. Dropping the options.\n";
    }
    b.seek(headerLength);

    if (totalLength > bufsize)
        EV << "Can not handle IPv4 packet of total length " << totalLength << "(captured only " << bufsize << " bytes).\n";

    dest->setByteLength(headerLength);
    unsigned int payloadLength = totalLength - headerLength;
    cPacket *encapPacket = nullptr;
    if (dest->getMoreFragments() || dest->getFragmentOffset() != 0) {  // IP fragment
        Buffer subBuffer(b, payloadLength);
        encapPacket = serializers.byteArraySerializer.deserialize(subBuffer, c);
        b.accessNBytes(subBuffer.getPos());
    }
    else
        encapPacket = SerializerBase::lookupAndDeserialize(b, c, IP_PROT, dest->getTransportProtocol(), payloadLength);

    if (encapPacket) {
        dest->encapsulate(encapPacket);
        dest->setName(encapPacket->getName());
    }
    return dest;
}

} // namespace serializer

} // namespace inet

