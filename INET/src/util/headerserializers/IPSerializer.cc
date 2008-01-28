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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

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

#if defined(_MSC_VER)
#undef s_addr   /* MSVC #definition interferes with us */
#endif

#ifndef _MSC_VER
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr


using namespace INETFw;



int IPSerializer::serialize(IPDatagram *dgram, unsigned char *buf, unsigned int bufsize)
{
    int packetLength;
    struct ip *ip = (struct ip *) buf;

    ip->ip_hl         = IP_HEADER_BYTES >> 2;
    ip->ip_v          = dgram->version();
    ip->ip_tos        = dgram->diffServCodePoint();
    ip->ip_id         = htons(dgram->identification());
    ip->ip_off        = htons(dgram->fragmentOffset());
    ip->ip_ttl        = dgram->timeToLive();
    ip->ip_p          = dgram->transportProtocol();
    ip->ip_src.s_addr = htonl(dgram->srcAddress().getInt());
    ip->ip_dst.s_addr = htonl(dgram->destAddress().getInt());
    ip->ip_sum        = 0;

    if (dgram->headerLength() > IP_HEADER_BYTES)
        EV << "Serializing an IP packet with options. Dropping the options.\n";

    packetLength = IP_HEADER_BYTES;

    cMessage *encapPacket = dgram->encapsulatedMsg();
    switch (dgram->transportProtocol())
    {
      case IP_PROT_ICMP:
        packetLength += ICMPSerializer().serialize(check_and_cast<ICMPMessage *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;
      case IP_PROT_UDP:
        packetLength += UDPSerializer().serialize(check_and_cast<UDPPacket *>(encapPacket),
                                                   buf+IP_HEADER_BYTES, bufsize-IP_HEADER_BYTES);
        break;
      default:
        opp_error("IPSerializer: cannot serialize protocol %d", dgram->transportProtocol());
    }

#ifdef linux
	ip->ip_len = htons(packetLength);
#else
	ip->ip_len = packetLength;
#endif
    return packetLength;
}

void IPSerializer::parse(unsigned char *buf, unsigned int bufsize, IPDatagram *dest)
{
    const struct ip *ip = (struct ip *) buf;
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

    if (headerLength > IP_HEADER_BYTES)
        EV << "Handling an captured IP packet with options. Dropping the options.\n";
    if (totalLength > bufsize)
        EV << "Can not handle IP packet of total length " << totalLength << "(captured only " << bufsize << " bytes).\n";
    dest->setByteLength(IP_HEADER_BYTES);

    cMessage *encapPacket = NULL;
    switch (dest->transportProtocol())
    {
      case IP_PROT_ICMP:
        encapPacket = new ICMPMessage("icmp-from-wire");
        ICMPSerializer().parse(buf + headerLength, min(totalLength, bufsize) - headerLength, (ICMPMessage *)encapPacket);
        break;
      case IP_PROT_UDP:
        encapPacket = new UDPPacket("udp-from-wire");
        UDPSerializer().parse(buf + headerLength, min(totalLength, bufsize) - headerLength, (UDPPacket *)encapPacket);
        break;
      default:
        opp_error("IPSerializer: cannot serialize protocol %d", dest->transportProtocol());
    }

    ASSERT(encapPacket);
    dest->encapsulate(encapPacket);
    dest->setName(encapPacket->name());
}

