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
#include "headers/igmp.h"
};
#include "IPv4Serializer.h"
#include "IGMPSerializer.h"
#include "TCPIPchecksum.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif


using namespace INETFw;


int IGMPSerializer::serialize(const IGMPMessage *pkt, unsigned char *buf, unsigned int bufsize)
{
    struct igmp *igmp = (struct igmp *) (buf);
    int packetLength;

    packetLength = IGMP_MINLEN;

    switch (pkt->getType())
    {
        case IGMP_MEMBERSHIP_QUERY:
            igmp->igmp_type = IGMP_MEMBERSHIP_QUERY;
            igmp->igmp_code = 0;
            igmp->igmp_cksum = 0;
            igmp->igmp_group.s_addr = htonl(check_and_cast<const IGMPQuery*>(pkt)->getGroupAddress().getInt());
            if (dynamic_cast<const IGMPv2Query*>(pkt))
            {
                igmp->igmp_code = dynamic_cast<const IGMPv2Query*>(pkt)->getMaxRespTime();
            }
            else if (dynamic_cast<const IGMPv3Query*>(pkt))
            {
                igmp->igmp_code = dynamic_cast<const IGMPv3Query*>(pkt)->getMaxRespCode();
                // TODO source list
            }
            break;
        case IGMPV1_MEMBERSHIP_REPORT:
            igmp->igmp_type = IGMP_V1_MEMBERSHIP_REPORT;
            igmp->igmp_code = 0;
            igmp->igmp_cksum = 0;
            igmp->igmp_group.s_addr = htonl(check_and_cast<const IGMPv1Report*>(pkt)->getGroupAddress().getInt());
            break;
        case IGMPV2_MEMBERSHIP_REPORT:
            igmp->igmp_type = IGMP_V2_MEMBERSHIP_REPORT;
            igmp->igmp_code = 0;
            igmp->igmp_cksum = 0;
            igmp->igmp_group.s_addr = htonl(check_and_cast<const IGMPv2Report*>(pkt)->getGroupAddress().getInt());
            break;
        case IGMPV2_LEAVE_GROUP:
            igmp->igmp_type = IGMP_V2_LEAVE_GROUP;
            igmp->igmp_code = 0;
            igmp->igmp_cksum = 0;
            igmp->igmp_group.s_addr = htonl(check_and_cast<const IGMPv2Leave*>(pkt)->getGroupAddress().getInt());
            break;
        case IGMPV3_MEMBERSHIP_REPORT:
            // TODO
        default:
            packetLength = 0;
            EV << "Can not serialize IGMP packet: type " << pkt->getType() << " not supported.";
            break;
    }
    igmp->igmp_cksum = TCPIPchecksum::checksum(buf, packetLength);
    return packetLength;
}

IGMPMessage *IGMPSerializer::parse(const unsigned char *buf, unsigned int bufsize)
{
    struct igmp *igmp = (struct igmp*) buf;

    switch (igmp->igmp_type)
    {
        case IGMP_MEMBERSHIP_QUERY:
            if (igmp->igmp_code == 0)
            {
                IGMPv1Query *pkt = new IGMPv1Query();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
                return pkt;
            }
            else
            {
                IGMPv2Query *pkt = new IGMPv2Query();
                pkt->setMaxRespTime(igmp->igmp_code);
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
                return pkt;
            }
            break;
        case IGMP_V1_MEMBERSHIP_REPORT:
            {
                IGMPv1Report *pkt = new IGMPv1Report();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
                return pkt;
            }
        case IGMP_V2_MEMBERSHIP_REPORT:
            {
                IGMPv2Report *pkt = new IGMPv2Report();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
                return pkt;
            }
        case IGMP_V2_LEAVE_GROUP:
            {
                IGMPv2Leave *pkt = new IGMPv2Leave();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
                return pkt;
            }
        default:
            EV << "Can not create IGMP packet: type " << igmp->igmp_type << " not supported.";
            break;
    }

    throw cRuntimeError("IGMPSerializer: can not create IGMP packet: type %d not supported", (int)igmp->igmp_type);
}
