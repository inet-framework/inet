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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6HeaderSerializer.h"
#include "inet/networklayer/ipv6/headers/ip6.h"

#if defined(_MSC_VER)
#undef s_addr    /* MSVC #definition interferes with us */
#endif // if defined(_MSC_VER)

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr

namespace inet {

Register_Serializer(Ipv6Header, Ipv6HeaderSerializer);

void Ipv6HeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ipv6Header = staticPtrCast<const Ipv6Header>(chunk);
    unsigned int i;
    uint32_t flowinfo;

    EV << "Serialize Ipv6 packet\n";

    B nextHdrCodePos = stream.getLength() + B(6);
    struct ip6_hdr ip6h;

    flowinfo = 0x06;
    flowinfo <<= 8;
    flowinfo |= ipv6Header->getTrafficClass();
    flowinfo <<= 20;
    flowinfo |= ipv6Header->getFlowLabel();
    ip6h.ip6_flow = htonl(flowinfo);
    ip6h.ip6_hlim = htons(ipv6Header->getHopLimit());

    ip6h.ip6_nxt = ipv6Header->getExtensionHeaderArraySize() != 0 ? ipv6Header->getExtensionHeader(0)->getExtensionType() : ipv6Header->getProtocolId();

    for (i = 0; i < 4; i++) {
        ip6h.ip6_src.__u6_addr.__u6_addr32[i] = htonl(ipv6Header->getSrcAddress().words()[i]);
    }
    for (i = 0; i < 4; i++) {
        ip6h.ip6_dst.__u6_addr.__u6_addr32[i] = htonl(ipv6Header->getDestAddress().words()[i]);
    }

    ip6h.ip6_plen = htons(B(ipv6Header->getPayloadLength()).get());

    stream.writeBytes((uint8_t *)&ip6h, IPv6_HEADER_BYTES);

    //FIXME serialize extension headers
    for (i = 0; i < ipv6Header->getExtensionHeaderArraySize(); i++) {
        const Ipv6ExtensionHeader *extHdr = ipv6Header->getExtensionHeader(i);
        stream.writeByte(i + 1 < ipv6Header->getExtensionHeaderArraySize() ? ipv6Header->getExtensionHeader(i + 1)->getExtensionType() : ipv6Header->getProtocolId());
        ASSERT((B(extHdr->getByteLength()).get() & 7) == 0);
        stream.writeByte(B(extHdr->getByteLength() - B(8)).get() / 8);
        switch (extHdr->getExtensionType()) {
            case IP_PROT_IPv6EXT_HOP: {
                const Ipv6HopByHopOptionsHeader *hdr = check_and_cast<const Ipv6HopByHopOptionsHeader *>(extHdr);
                stream.writeByteRepeatedly(0, B(hdr->getByteLength()).get() - 2);    //TODO
                break;
            }
            case IP_PROT_IPv6EXT_DEST: {
                const Ipv6DestinationOptionsHeader *hdr = check_and_cast<const Ipv6DestinationOptionsHeader *>(extHdr);
                stream.writeByteRepeatedly(0, B(hdr->getByteLength()).get() - 2);    //TODO
                break;
            }
            case IP_PROT_IPv6EXT_ROUTING: {
                const Ipv6RoutingHeader *hdr = check_and_cast<const Ipv6RoutingHeader *>(extHdr);
                stream.writeByte(hdr->getRoutingType());
                stream.writeByte(hdr->getSegmentsLeft());
                for (unsigned int j = 0; j < hdr->getAddressArraySize(); j++) {
                    stream.writeIpv6Address(hdr->getAddress(j));
                }
                stream.writeByteRepeatedly(0, 4);
                break;
            }
            case IP_PROT_IPv6EXT_FRAGMENT: {
                const Ipv6FragmentHeader *hdr = check_and_cast<const Ipv6FragmentHeader *>(extHdr);
                ASSERT((hdr->getFragmentOffset() & 7) == 0);
                stream.writeUint16Be(hdr->getFragmentOffset() | (hdr->getMoreFragments() ? 1 : 0));
                stream.writeUint32Be(hdr->getIdentification());
                break;
            }
            case IP_PROT_IPv6EXT_AUTH: {
                const Ipv6AuthenticationHeader *hdr = check_and_cast<const Ipv6AuthenticationHeader *>(extHdr);
                stream.writeByteRepeatedly(0, B(hdr->getByteLength()).get() - 2);    //TODO
                break;
            }
            case IP_PROT_IPv6EXT_ESP: {
                const Ipv6EncapsulatingSecurityPayloadHeader *hdr = check_and_cast<const Ipv6EncapsulatingSecurityPayloadHeader *>(extHdr);
                stream.writeByteRepeatedly(0, B(hdr->getByteLength()).get() - 2);    //TODO
                break;
            }
            default: {
                throw cRuntimeError("Unknown Ipv6 extension header %d (%s)%s", extHdr->getExtensionType(), extHdr->getClassName(), extHdr->getFullName());
                break;
            }
        }
        ASSERT(nextHdrCodePos + extHdr->getByteLength() == stream.getLength());
    }
}

const Ptr<Chunk> Ipv6HeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t buffer[B(IPv6_HEADER_BYTES).get()];
    stream.readBytes(buffer, IPv6_HEADER_BYTES);
    auto dest = makeShared<Ipv6Header>();
    const struct ip6_hdr& ip6h = *static_cast<const struct ip6_hdr *>((void *)&buffer);
    uint32_t flowinfo = ntohl(ip6h.ip6_flow);
    dest->setFlowLabel(flowinfo & 0xFFFFF);
    flowinfo >>= 20;
    dest->setTrafficClass(flowinfo & 0xFF);

    dest->setProtocolId(static_cast<IpProtocolId>(ip6h.ip6_nxt));
    dest->setHopLimit(ntohs(ip6h.ip6_hlim));

    Ipv6Address temp;
    temp.set(ntohl(ip6h.ip6_src.__u6_addr.__u6_addr32[0]),
             ntohl(ip6h.ip6_src.__u6_addr.__u6_addr32[1]),
             ntohl(ip6h.ip6_src.__u6_addr.__u6_addr32[2]),
             ntohl(ip6h.ip6_src.__u6_addr.__u6_addr32[3]));
    dest->setSrcAddress(temp);

    temp.set(ntohl(ip6h.ip6_dst.__u6_addr.__u6_addr32[0]),
             ntohl(ip6h.ip6_dst.__u6_addr.__u6_addr32[1]),
             ntohl(ip6h.ip6_dst.__u6_addr.__u6_addr32[2]),
             ntohl(ip6h.ip6_dst.__u6_addr.__u6_addr32[3]));
    dest->setDestAddress(temp);
    dest->setPayloadLength(B(ip6h.ip6_plen));

    return dest;
}

} // namespace inet

