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

#include "inet/common/serializer/ipv4/IGMPSerializer.h"

#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/ipv4/headers/ip.h"
#include "inet/common/serializer/ipv4/headers/igmp.h"

#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#include "inet/common/serializer/TCPIPchecksum.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

int IGMPSerializer::serialize(const IGMPMessage *pkt, unsigned char *buf, unsigned int bufsize)
{
    struct igmp *igmp = (struct igmp *)(buf);
    int packetLength;

    packetLength = IGMP_MINLEN;

    switch (pkt->getType()) {
        case IGMP_V1_MEMBERSHIP_REPORT:
        case IGMP_V2_MEMBERSHIP_REPORT:
        case IGMP_V2_LEAVE_GROUP:
        case IGMP_MEMBERSHIP_QUERY:
            igmp->igmp_type = pkt->getType();
            igmp->igmp_code = pkt->getMaxRespTime();
            igmp->igmp_cksum = 0;
            igmp->igmp_group.s_addr = htonl(pkt->getGroupAddress().getInt());
            break;

        default:
            packetLength = 0;
            EV << "Can not serialize IGMP packet: type " << pkt->getType() << " not supported.";
            break;
    }
    igmp->igmp_cksum = TCPIPchecksum::checksum(buf, packetLength);
    return packetLength;
}

void IGMPSerializer::parse(const unsigned char *buf, unsigned int bufsize, IGMPMessage *pkt)
{
    struct igmp *igmp = (struct igmp *)buf;

    switch (igmp->igmp_type) {
        case IGMP_V1_MEMBERSHIP_REPORT:
        case IGMP_V2_MEMBERSHIP_REPORT:
        case IGMP_V2_LEAVE_GROUP:
        case IGMP_MEMBERSHIP_QUERY:
            pkt->setType(igmp->igmp_type);
            pkt->setMaxRespTime(igmp->igmp_code);
            pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
            break;

        default:
            EV << "Can not create IGMP packet: type " << igmp->igmp_type << " not supported.";
            break;
    }
}

} // namespace serializer

} // namespace inet

