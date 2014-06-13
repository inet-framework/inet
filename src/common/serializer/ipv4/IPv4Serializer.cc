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

#include <algorithm> // std::min
#include <platdep/sockets.h>

#include "headers/defs.h"

namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
{
#include "headers/bsdint.h"
#include "headers/in.h"
#include "headers/in_systm.h"
#include "headers/ip.h"
};

#include "IPv4Serializer.h"

#include "ICMPSerializer.h"
#include "IGMPSerializer.h"
#include "IPProtocolId_m.h"

#ifdef WITH_UDP
#include "UDPSerializer.h"
#endif

#ifdef WITH_SCTP
#include "SCTPSerializer.h"    //I.R.
#endif

#include "TCPIPchecksum.h"

#ifdef WITH_TCP_COMMON
#include "TCPSegment.h"
#include "TCPSerializer.h"    //I.R.
#endif

#if defined(_MSC_VER)
#undef s_addr   /* MSVC #definition interferes with us */
#endif

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr


using namespace INETFw;



int IPv4Serializer::serialize(const IPv4Datagram *dgram, unsigned char *buf, unsigned int bufsize, bool hasCalcChkSum)
{
    int packetLength;
    struct ip *ip = (struct ip *) buf;

    ip->ip_hl = IP_HEADER_BYTES >> 2;
    ip->ip_v = dgram->getVersion();
    ip->ip_tos = dgram->getTypeOfService();
    ip->ip_id = htons(dgram->getIdentification());
    uint16_t ip_off = dgram->getFragmentOffset()/8;
    if (dgram->getMoreFragments())
        ip_off |= IP_MF;
    if (dgram->getDontFragment())
        ip_off |= IP_DF;
    ip->ip_off = htons(ip_off);
    ip->ip_ttl = dgram->getTimeToLive();
    ip->ip_p = dgram->getTransportProtocol();
    ip->ip_src.s_addr = htonl(dgram->getSrcAddress().getInt());
    ip->ip_dst.s_addr = htonl(dgram->getDestAddress().getInt());
    ip->ip_sum = 0;

    if (dgram->getHeaderLength() > IP_HEADER_BYTES)
        EV << "Serializing an IPv4 packet with options. Dropping the options.\n";

    packetLength = IP_HEADER_BYTES;

    cMessage *encapPacket = dgram->getEncapsulatedPacket();

    switch (dgram->getTransportProtocol())
    {
      case IP_PROT_ICMP:
        packetLength += ICMPSerializer().serialize(check_and_cast<ICMPMessage *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;

      case IP_PROT_IGMP:
        packetLength += IGMPSerializer().serialize(check_and_cast<IGMPMessage *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;

#ifdef WITH_UDP
      case IP_PROT_UDP:
        packetLength += UDPSerializer().serialize(check_and_cast<UDPPacket *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;
#endif

#ifdef WITH_SCTP
      case IP_PROT_SCTP:    //I.R.
        packetLength += SCTPSerializer().serialize(check_and_cast<SCTPMessage *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;
#endif

#ifdef WITH_TCP_COMMON
      case IP_PROT_TCP:        //I.R.
        packetLength += TCPSerializer().serialize(check_and_cast<TCPSegment *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES,
                                                   dgram->getSrcAddress(), dgram->getDestAddress());
        break;
#endif

      default:
        throw cRuntimeError(dgram, "IPv4Serializer: cannot serialize protocol %d", dgram->getTransportProtocol());
    }

    ip->ip_len = htons(packetLength);

    if (hasCalcChkSum)
    {
        ip->ip_sum = TCPIPchecksum::checksum(buf, IP_HEADER_BYTES);
    }

    return packetLength;
}

void IPv4Serializer::parse(const unsigned char *buf, unsigned int bufsize, IPv4Datagram *dest)
{
    const struct ip *ip = (const struct ip *) buf;
    unsigned int totalLength, headerLength;

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
    dest->setFragmentOffset((ntohs(ip->ip_off) & IP_OFFMASK)*8);
    dest->setTypeOfService(ip->ip_tos);
    totalLength = ntohs(ip->ip_len);
    headerLength = ip->ip_hl << 2;

    if (headerLength > (unsigned int)IP_HEADER_BYTES)
        EV << "Handling an captured IPv4 packet with options. Dropping the options.\n";

    if (totalLength > bufsize)
        EV << "Can not handle IPv4 packet of total length " << totalLength << "(captured only " << bufsize << " bytes).\n";

    dest->setByteLength(IP_HEADER_BYTES);

    cPacket *encapPacket = NULL;
    unsigned int encapLength = std::min(totalLength, bufsize) - headerLength;

    switch (dest->getTransportProtocol())
    {
      case IP_PROT_ICMP:
        encapPacket = new ICMPMessage("icmp-from-wire");
        ICMPSerializer().parse(buf + headerLength, encapLength, (ICMPMessage *)encapPacket);
        break;

      case IP_PROT_IGMP:
        encapPacket = new IGMPMessage("igmp-from-wire");
        IGMPSerializer().parse(buf + headerLength, encapLength, (IGMPMessage *)encapPacket);
        break;

#ifdef WITH_UDP
      case IP_PROT_UDP:
        encapPacket = new UDPPacket("udp-from-wire");
        UDPSerializer().parse(buf + headerLength, encapLength, (UDPPacket *)encapPacket);
        break;
#endif

#ifdef WITH_SCTP
      case IP_PROT_SCTP:
        encapPacket = new SCTPMessage("sctp-from-wire");
        SCTPSerializer().parse(buf + headerLength, encapLength, (SCTPMessage *)encapPacket);
        break;
#endif

#ifdef WITH_TCP_COMMON
      case IP_PROT_TCP:
        encapPacket = new TCPSegment("tcp-from-wire");
        TCPSerializer().parse(buf + headerLength, encapLength, (TCPSegment *)encapPacket, true);
        break;
#endif

      default:
        throw cRuntimeError("IPv4Serializer: cannot parse protocol %d", dest->getTransportProtocol());
    }

    ASSERT(encapPacket);
    dest->encapsulate(encapPacket);
    dest->setName(encapPacket->getName());
}

