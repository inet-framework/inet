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

#include <platdep/sockets.h>

#include "headers/defs.h"

namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
{
#include "headers/bsdint.h"
#include "headers/in.h"
#include "headers/in_systm.h"
#include "headers/udp.h"
};

#include "UDPSerializer.h"

#include "ByteArrayMessage.h"
#include "TCPIPchecksum.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

#define SOURCE_IP_HEADER 8
#define DEST_IP_HEADER 4
#define PROTOCOL_IP_HEADER 4

using namespace INETFw;

struct pseudo_header
{
    int32 source;
    int32 dest;
    uint8 reserve;
    uint8 protocol;
    int16 udpLen;
};

int UDPSerializer::serialize(const UDPPacket *pkt, unsigned char *buf, unsigned int bufsize)
{
    struct udphdr *udphdr = (struct udphdr *) (buf);
    int16 packetLength;
    char *check_sum;
    struct pseudo_header* psh;

    packetLength = (int16)pkt->getByteLength();

    ByteArrayMessage *encapPacket = new ByteArrayMessage("Payload-from-wire");
    encapPacket = (ByteArrayMessage*)(pkt->getEncapsulatedPacket());
    memcpy(buf+sizeof(struct udphdr),encapPacket->getFullName(),packetLength - sizeof(struct udphdr));

    udphdr->uh_sport = htons(pkt->getSourcePort());
    udphdr->uh_dport = htons(pkt->getDestinationPort());
    udphdr->uh_ulen = htons(packetLength);

    check_sum = (char*)malloc(sizeof(struct pseudo_header) + packetLength);
    psh = (struct pseudo_header*)check_sum;

    //To build the pseudo header, it needs the source and dest addresses from the ip header,
    //as well the protocol, for this reason memcpy copies from buf-SOURCE_IP_HEADER, buf-DEST_IP_HEADER
    //and buf-PROTOCOL_IP_HEADER
    memcpy(&(psh->source),buf-SOURCE_IP_HEADER,4);
    memcpy(&(psh->dest),buf-DEST_IP_HEADER,4);
    memset(&(psh->reserve),0,1);
    memcpy(&(psh->protocol),buf-PROTOCOL_IP_HEADER,1);
    psh->udpLen = udphdr->uh_ulen;
    memcpy(check_sum+sizeof(struct pseudo_header), buf, packetLength);

    udphdr->uh_sum = TCPIPchecksum::checksum(check_sum,sizeof(struct pseudo_header) + packetLength);

    return packetLength;
}

void UDPSerializer::parse(const unsigned char *buf, unsigned int bufsize, UDPPacket *dest)
{
    struct udphdr *udphdr = (struct udphdr*) buf;

    dest->setSourcePort(ntohs(udphdr->uh_sport));
    dest->setDestinationPort(ntohs(udphdr->uh_dport));
    dest->setByteLength(8);
    ByteArrayMessage *encapPacket = new ByteArrayMessage("Payload-from-wire");
    encapPacket->setDataFromBuffer(buf + sizeof(struct udphdr), ntohs(udphdr->uh_ulen) - sizeof(struct udphdr));
    encapPacket->addByteLength(ntohs(udphdr->uh_ulen) - sizeof(struct udphdr));
    encapPacket->setName((const char *)buf + sizeof(struct udphdr));
    dest->encapsulate(encapPacket);
    dest->setName(encapPacket->getName());
}
