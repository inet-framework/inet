//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
// Copyright (C) 2009 Thomas Reschka
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

#include "IPSerializer.h"
#include "ICMPSerializer.h"
#include "UDPSerializer.h"
#include "SCTPSerializer.h"    //I.R.
#include "TCPSerializer.h"    //I.R.

#if defined(_MSC_VER)
#undef s_addr   /* MSVC #definition interferes with us */
#endif

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

#ifndef IP_PROT_SCTP    //I.R.
#define IP_PROT_SCTP 132
#endif

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr


using namespace INETFw;



int IPSerializer::serialize(const IPDatagram *dgram, unsigned char *buf, unsigned int bufsize)
{
    int packetLength;
    struct ip *ip = (struct ip *) buf;

    ip->ip_hl         = IP_HEADER_BYTES >> 2;
    ip->ip_v          = dgram->getVersion();
    ip->ip_tos        = dgram->getDiffServCodePoint();
    ip->ip_id         = htons(dgram->getIdentification());
    ip->ip_off        = htons(dgram->getFragmentOffset());
    ip->ip_ttl        = dgram->getTimeToLive();
    ip->ip_p          = dgram->getTransportProtocol();
    ip->ip_src.s_addr = htonl(dgram->getSrcAddress().getInt());
    ip->ip_dst.s_addr = htonl(dgram->getDestAddress().getInt());
    ip->ip_sum        = 0;

    if (dgram->getHeaderLength() > IP_HEADER_BYTES)
        EV << "Serializing an IP packet with options. Dropping the options.\n";

    packetLength = IP_HEADER_BYTES;

    cMessage *encapPacket = dgram->getEncapsulatedPacket();
    switch (dgram->getTransportProtocol())
    {
      case IP_PROT_ICMP:
        packetLength += ICMPSerializer().serialize(check_and_cast<ICMPMessage *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;
      case IP_PROT_UDP:
        packetLength += UDPSerializer().serialize(check_and_cast<UDPPacket *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;
      case IP_PROT_SCTP:    //I.R.
        packetLength += SCTPSerializer().serialize(check_and_cast<SCTPMessage *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;
      case IP_PROT_TCP:        //I.R.
        packetLength += TCPSerializer().serialize(check_and_cast<TCPSegment *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES,
                                                   dgram->getSrcAddress(), dgram->getDestAddress());
        break;
      default:
        opp_error("IPSerializer: cannot serialize protocol %d", dgram->getTransportProtocol());
    }

    ip->ip_len = htons(packetLength);

    return packetLength;
}

void IPSerializer::parse(const unsigned char *buf, unsigned int bufsize, IPDatagram *dest)
{
    const struct ip *ip = (const struct ip *) buf;
    unsigned int totalLength, headerLength;

    dest->setVersion(ip->ip_v);
    dest->setHeaderLength(IP_HEADER_BYTES);
    dest->setSrcAddress(ntohl(ip->ip_src.s_addr));
    dest->setDestAddress(ntohl(ip->ip_dst.s_addr));
    dest->setTransportProtocol(ip->ip_p);
    dest->setTimeToLive(ip->ip_ttl);
    dest->setIdentification(ntohs(ip->ip_id));
    dest->setMoreFragments((ip->ip_off) & !IP_OFFMASK & IP_MF);
    dest->setDontFragment((ip->ip_off) & !IP_OFFMASK & IP_DF);
    dest->setFragmentOffset((ntohs(ip->ip_off)) & IP_OFFMASK);
    dest->setDiffServCodePoint(ip->ip_tos);
    totalLength = ntohs(ip->ip_len);
    headerLength = ip->ip_hl << 2;

    if (headerLength > (unsigned int)IP_HEADER_BYTES)
        EV << "Handling an captured IP packet with options. Dropping the options.\n";
    if (totalLength > bufsize)
        EV << "Can not handle IP packet of total length " << totalLength << "(captured only " << bufsize << " bytes).\n";
    dest->setByteLength(IP_HEADER_BYTES);

    cPacket *encapPacket = NULL;
    switch (dest->getTransportProtocol())
    {
      case IP_PROT_ICMP:
        encapPacket = new ICMPMessage("icmp-from-wire");
        ICMPSerializer().parse(buf + headerLength, std::min(totalLength, bufsize) - headerLength, (ICMPMessage *)encapPacket);
        break;
      case IP_PROT_UDP:
        encapPacket = new UDPPacket("udp-from-wire");
        UDPSerializer().parse(buf + headerLength, std::min(totalLength, bufsize) - headerLength, (UDPPacket *)encapPacket);
        break;
      case IP_PROT_SCTP:
        encapPacket = new SCTPMessage("sctp-from-wire");
        SCTPSerializer().parse(buf + headerLength, (unsigned int)(std::min(totalLength, bufsize) - headerLength), (SCTPMessage *)encapPacket);
        break;
      case IP_PROT_TCP:
        encapPacket = new TCPSegment("tcp-from-wire");
        TCPSerializer().parse(buf + headerLength, (unsigned int)(std::min(totalLength, bufsize) - headerLength), (TCPSegment *)encapPacket);
        break;
      default:
        opp_error("IPSerializer: cannot serialize protocol %d", dest->getTransportProtocol());
    }

    ASSERT(encapPacket);
    dest->encapsulate(encapPacket);
    dest->setName(encapPacket->getName());
}
