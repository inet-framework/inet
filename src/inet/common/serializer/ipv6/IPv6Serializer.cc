//
// Copyright (C) 2013 Irene Ruengeler
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

#include "inet/common/serializer/SerializerUtil.h"

#include "inet/common/serializer/ipv6/IPv6Serializer.h"

#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/headers/ethernethdr.h"
#include "inet/common/serializer/ipv6/headers/ip6.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"


#include "inet/common/serializer/TCPIPchecksum.h"

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/common/serializer/tcp/TCPSerializer.h"
#endif // ifdef WITH_TCP_COMMON

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

Register_Serializer(IPv6Datagram, ETHERTYPE, ETHERTYPE_IPv6, IPv6Serializer);

void IPv6Serializer::serialize(const cPacket *pkt, Buffer &b, Context& c)
{
    const IPv6Datagram *dgram = check_and_cast<const IPv6Datagram *>(pkt);
    unsigned int i;
    uint32_t flowinfo;

    EV << "Serialize IPv6 packet\n";

    unsigned int nextHdrCodePos = b.getPos() + 6;
    struct ip6_hdr *ip6h = (struct ip6_hdr *)b.accessNBytes(sizeof(struct ip6_hdr));

    flowinfo = 0x06;
    flowinfo <<= 8;
    flowinfo |= dgram->getTrafficClass();
    flowinfo <<= 20;
    flowinfo |= dgram->getFlowLabel();
    ip6h->ip6_flow = htonl(flowinfo);
    ip6h->ip6_hlim = htons(dgram->getHopLimit());

    ip6h->ip6_nxt = dgram->getTransportProtocol();

    for (i = 0; i < 4; i++) {
        ip6h->ip6_src.__u6_addr.__u6_addr32[i] = htonl(dgram->getSrcAddress().words()[i]);
    }
    for (i = 0; i < 4; i++) {
        ip6h->ip6_dst.__u6_addr.__u6_addr32[i] = htonl(dgram->getDestAddress().words()[i]);
    }
    c.l3AddressesPtr = &ip6h->ip6_src.__u6_addr.__u6_addr32[0];
    c.l3AddressesLength = 32;

    //FIXME serialize extension headers
    for (i = 0; i < dgram->getExtensionHeaderArraySize(); i++) {
        const IPv6ExtensionHeader *extHdr = dgram->getExtensionHeader(i);
        b.writeByteTo(nextHdrCodePos, extHdr->getExtensionType());
        nextHdrCodePos = b.getPos();
        b.writeByte(dgram->getTransportProtocol());
        ASSERT((extHdr->getByteLength() & 7) == 0);
        b.writeByte((extHdr->getByteLength() - 8) / 8);
        switch (extHdr->getExtensionType()) {
            case IP_PROT_IPv6EXT_HOP: {
                const IPv6HopByHopOptionsHeader *hdr = check_and_cast<const IPv6HopByHopOptionsHeader *>(extHdr);
                b.fillNBytes(hdr->getByteLength() - 2, '\0');    //TODO
                break;
            }
            case IP_PROT_IPv6EXT_DEST: {
                const IPv6DestinationOptionsHeader *hdr = check_and_cast<const IPv6DestinationOptionsHeader *>(extHdr);
                b.fillNBytes(hdr->getByteLength() - 2, '\0');    //TODO
                break;
            }
            case IP_PROT_IPv6EXT_ROUTING: {
                const IPv6RoutingHeader *hdr = check_and_cast<const IPv6RoutingHeader *>(extHdr);
                b.writeByte(hdr->getRoutingType());
                b.writeByte(hdr->getSegmentsLeft());
                for (unsigned int j = 0; j < hdr->getAddressArraySize(); j++) {
                    b.writeIPv6Address(hdr->getAddress(j));
                }
                b.fillNBytes(4, '\0');
                break;
            }
            case IP_PROT_IPv6EXT_FRAGMENT: {
                const IPv6FragmentHeader *hdr = check_and_cast<const IPv6FragmentHeader *>(extHdr);
                ASSERT((hdr->getFragmentOffset() & 7) == 0);
                b.writeUint16(hdr->getFragmentOffset() | (hdr->getMoreFragments() ? 1 : 0));
                b.writeUint32(hdr->getIdentification());
                break;
            }
            case IP_PROT_IPv6EXT_AUTH: {
                const IPv6AuthenticationHeader *hdr = check_and_cast<const IPv6AuthenticationHeader *>(extHdr);
                b.fillNBytes(hdr->getByteLength() - 2, '\0');    //TODO
                break;
            }
            case IP_PROT_IPv6EXT_ESP: {
                const IPv6EncapsulatingSecurityPayloadHeader *hdr = check_and_cast<const IPv6EncapsulatingSecurityPayloadHeader *>(extHdr);
                b.fillNBytes(hdr->getByteLength() - 2, '\0');    //TODO
                break;
            }
            default: {
                throw cRuntimeError("Unknown IPv6 extension header %d (%s)%s", extHdr->getExtensionType(), extHdr->getClassName(), extHdr->getFullName());
                break;
            }
        }
        ASSERT(nextHdrCodePos + extHdr->getByteLength() == b.getPos());
    }

    const cPacket *encapPacket = dgram->getEncapsulatedPacket();
    unsigned int encapStart = b.getPos();
    SerializerBase::lookupAndSerialize(encapPacket, b, c, IP_PROT, dgram->getTransportProtocol());
    unsigned int encapEnd = b.getPos();

    ip6h->ip6_plen = htons(encapEnd - encapStart);
}

cPacket* IPv6Serializer::deserialize(const Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);
    IPv6Datagram *dest = new IPv6Datagram("parsed-ipv6");
    const struct ip6_hdr *ip6h = (struct ip6_hdr *) b.accessNBytes(IPv6_HEADER_BYTES);
    uint32_t flowinfo = ntohl(ip6h->ip6_flow);
    dest->setFlowLabel(flowinfo & 0xFFFFF);
    flowinfo >>= 20;
    dest->setTrafficClass(flowinfo & 0xFF);

    unsigned int packetLength = ntohs(ip6h->ip6_plen);

    dest->setTransportProtocol(ip6h->ip6_nxt);
    dest->setHopLimit(ntohs(ip6h->ip6_hlim));

    IPv6Address temp;
    temp.set(ntohl(ip6h->ip6_src.__u6_addr.__u6_addr32[0]),
             ntohl(ip6h->ip6_src.__u6_addr.__u6_addr32[1]),
             ntohl(ip6h->ip6_src.__u6_addr.__u6_addr32[2]),
             ntohl(ip6h->ip6_src.__u6_addr.__u6_addr32[3]));
    dest->setSrcAddress(temp);

    temp.set(ntohl(ip6h->ip6_dst.__u6_addr.__u6_addr32[0]),
             ntohl(ip6h->ip6_dst.__u6_addr.__u6_addr32[1]),
             ntohl(ip6h->ip6_dst.__u6_addr.__u6_addr32[2]),
             ntohl(ip6h->ip6_dst.__u6_addr.__u6_addr32[3]));
    dest->setDestAddress(temp);

    c.l3AddressesPtr = &ip6h->ip6_src.__u6_addr.__u6_addr32[0];
    c.l3AddressesLength = 32;

    if (packetLength + IPv6_HEADER_BYTES > b._getBufSize()) {
        EV_ERROR << "Can not handle IPv6 packet of total length " << packetLength + IPv6_HEADER_BYTES << "(captured only " << b._getBufSize() << " bytes).\n";
        b.setError();
    }
    dest->setByteLength(b.getPos());    // set header size

    cPacket *encapPacket = SerializerBase::lookupAndDeserialize(b, c, IP_PROT, dest->getTransportProtocol(), packetLength);
    if (encapPacket) {
        dest->encapsulate(encapPacket);
        dest->setName(encapPacket->getName());
    }
    return dest;
}

} // namespace serializer

} // namespace inet

