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
#include <platdep/sockets.h>

#include "inet/common/serializer/ipv6/IPv6Serializer.h"

#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/ipv6/headers/ip6.h"

#ifdef WITH_UDP
#include "inet/common/serializer/udp/UDPSerializer.h"
#endif // ifdef WITH_UDP

#ifdef WITH_SCTP
#include "inet/common/serializer/sctp/SCTPSerializer.h"
#endif // ifdef WITH_SCTP

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

int IPv6Serializer::serialize(const IPv6Datagram *dgram, unsigned char *buf, unsigned int bufsize)
{
    int packetLength, i;
    uint32_t flowinfo;

    EV << "Serialize IPv6 packet\n";

    struct ip6_hdr *ip6h = (struct ip6_hdr *)buf;

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

    cMessage *encapPacket = dgram->getEncapsulatedPacket();

    switch (dgram->getTransportProtocol()) {
/*      case IP_PROT_IPv6_ICMP:
        packetLength += ICMPv6Serializer().serialize(check_and_cast<ICMPv6Message *>(encapPacket),
                                                   buf+IPv6_HEADER_BYTES, bufsize-IPv6_HEADER_BYTES);
        break;*/
#ifdef WITH_UDP
        case IP_PROT_UDP:
            packetLength = UDPSerializer().serialize(check_and_cast<UDPPacket *>(encapPacket),
                        buf + IPv6_HEADER_BYTES, bufsize - IPv6_HEADER_BYTES);
            break;
#endif // ifdef WITH_UDP

#ifdef WITH_SCTP
        case IP_PROT_SCTP:
            packetLength = sctp::SCTPSerializer().serialize(check_and_cast<sctp::SCTPMessage *>(encapPacket),
                        buf + IPv6_HEADER_BYTES, bufsize - IPv6_HEADER_BYTES);
            break;
#endif // ifdef WITH_SCTP

#ifdef WITH_TCP_COMMON
        case IP_PROT_TCP:
            packetLength = TCPSerializer().serialize(check_and_cast<tcp::TCPSegment *>(encapPacket),
                        buf + IPv6_HEADER_BYTES, bufsize - IPv6_HEADER_BYTES,
                        dgram->getSrcAddress(), dgram->getDestAddress());
            break;
#endif // ifdef WITH_TCP_COMMON

        default:
            printf("IPv6Serializer: cannot serialize protocol %d\n", dgram->getTransportProtocol());
            return -1;
    }

    ip6h->ip6_plen = htons(packetLength);

    return packetLength + IPv6_HEADER_BYTES;
}

} // namespace serializer

} // namespace inet

