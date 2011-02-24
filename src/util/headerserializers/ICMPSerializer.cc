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
#include "headers/ip.h"
#include "headers/ip_icmp.h"
};
#include "IPSerializer.h"
#include "ICMPSerializer.h"
#include "PingPayload_m.h"
#include "TCPIPchecksum.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif


using namespace INETFw;


int ICMPSerializer::serialize(const ICMPMessage *pkt, unsigned char *buf, unsigned int bufsize)
{
    struct icmp *icmp = (struct icmp *) (buf);
    int packetLength;

    packetLength = ICMP_MINLEN;

    switch(pkt->getType())
    {
        case ICMP_ECHO_REQUEST:
        {
            PingPayload *pp = check_and_cast<PingPayload* >(pkt->getEncapsulatedPacket());
            icmp->icmp_type = ICMP_ECHO;
            icmp->icmp_code = 0;
            icmp->icmp_id   = htons(pp->getOriginatorId());
            icmp->icmp_seq  = htons(pp->getSeqNo());
            unsigned int datalen = (pp->getByteLength() - 4);
            for (unsigned int i=0; i < datalen; i++)
                if (i < pp->getDataArraySize()) {
                    icmp->icmp_data[i] = pp->getData(i);
                } else {
                    icmp->icmp_data[i] = 'a';
                }
            packetLength += datalen;
            break;
        }
        case ICMP_ECHO_REPLY:
        {
            PingPayload *pp = check_and_cast<PingPayload* >(pkt->getEncapsulatedPacket());
            icmp->icmp_type = ICMP_ECHOREPLY;
            icmp->icmp_code = 0;
            icmp->icmp_id   = htons(pp->getOriginatorId());
            icmp->icmp_seq  = htons(pp->getSeqNo());
            unsigned int datalen = pp->getDataArraySize();
            for(unsigned int i=0; i < datalen; i++)
                icmp->icmp_data[i] = pp->getData(i);
            packetLength += datalen;
            break;
        }
        case ICMP_DESTINATION_UNREACHABLE:
        {
            IPDatagram *ip = check_and_cast<IPDatagram* >(pkt->getEncapsulatedPacket());
            icmp->icmp_type = ICMP_UNREACH;
            icmp->icmp_code = pkt->getCode();
            packetLength += IPSerializer().serialize(ip, (unsigned char *)icmp->icmp_data, bufsize - ICMP_MINLEN);
            break;
        }
        case ICMP_TIME_EXCEEDED:
        {
            IPDatagram *ip = check_and_cast<IPDatagram* >(pkt->getEncapsulatedPacket());
            icmp->icmp_type = ICMP_TIMXCEED;
            icmp->icmp_code = ICMP_TIMXCEED_INTRANS;
            packetLength += IPSerializer().serialize(ip, (unsigned char *)icmp->icmp_data, bufsize - ICMP_MINLEN);
            break;
        }
        default:
        {
            packetLength = 0;
            EV << "Can not serialize ICMP packet: type " << pkt->getType() << " not supported.";
            break;
        }
    }
    icmp->icmp_cksum = TCPIPchecksum::checksum(buf, packetLength);
    return packetLength;
}

void ICMPSerializer::parse(const unsigned char *buf, unsigned int bufsize, ICMPMessage *pkt)
{
    struct icmp *icmp = (struct icmp*) buf;

    switch(icmp->icmp_type)
    {
        case ICMP_ECHO:
        {
            PingPayload *pp;
            char name[32];

            pkt->setType(ICMP_ECHO_REQUEST);
            pkt->setCode(0);
            pkt->setByteLength(4);
            sprintf(name,"ping%d", ntohs(icmp->icmp_seq));
            pp = new PingPayload(name);
            pp->setOriginatorId(ntohs(icmp->icmp_id));
            pp->setSeqNo(ntohs(icmp->icmp_seq));
            pp->setByteLength(bufsize - 4);
            pp->setDataArraySize(bufsize - ICMP_MINLEN);
            for(unsigned int i=0; i<bufsize - ICMP_MINLEN; i++)
                pp->setData(i, icmp->icmp_data[i]);
            pkt->encapsulate(pp);
            pkt->setName(pp->getName());
            break;
        }
        case ICMP_ECHOREPLY:
        {
            PingPayload *pp;
            char name[32];

            pkt->setType(ICMP_ECHO_REPLY);
            pkt->setCode(0);
            pkt->setByteLength(4);
            sprintf(name,"ping%d-reply", ntohs(icmp->icmp_seq));
            pp = new PingPayload(name);
            pp->setOriginatorId(ntohs(icmp->icmp_id));
            pp->setSeqNo(ntohs(icmp->icmp_seq));
            pp->setByteLength(bufsize - 4);
            pp->setDataArraySize(bufsize - ICMP_MINLEN);
            for (unsigned int i=0; i<bufsize - ICMP_MINLEN; i++)
                pp->setData(i, icmp->icmp_data[i]);
            pkt->encapsulate(pp);
            pkt->setName(pp->getName());
            break;
        }
        default:
        {
            EV << "Can not create ICMP packet: type " << icmp->icmp_type << " not supported.";
            break;
        }
    }
}
