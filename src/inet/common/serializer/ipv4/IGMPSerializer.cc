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
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/ipv4/IGMPMessage.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

Register_Serializer(IGMPMessage, IP_PROT, IP_PROT_IGMP, IGMPSerializer);

void IGMPSerializer::serialize(const cPacket *_pkt, Buffer &b, Context& context)
{
    unsigned int startPos = b.getPos();
    const IGMPMessage *pkt = check_and_cast<const IGMPMessage *>(_pkt);
    struct igmp *igmp = (struct igmp *)(b.accessNBytes(sizeof(struct igmp)));

    switch (pkt->getType())
    {
        case IGMP_MEMBERSHIP_QUERY:
            igmp->igmp_type = IGMP_MEMBERSHIP_QUERY;
            igmp->igmp_code = 0;
            igmp->igmp_cksum = 0;
            igmp->igmp_group.s_addr = htonl(check_and_cast<const IGMPQuery*>(pkt)->getGroupAddress().getInt());
            if (dynamic_cast<const IGMPv2Query*>(pkt))
            {
                igmp->igmp_code = static_cast<const IGMPv2Query*>(pkt)->getMaxRespTime();
            }
            else if (dynamic_cast<const IGMPv3Query*>(pkt))
            {
                igmp->igmp_code = static_cast<const IGMPv3Query*>(pkt)->getMaxRespCode();
                // TODO source list
            }
            else
                throw cRuntimeError("unknown IGMP_MEMBERSHIP_QUERY: %s", pkt->getClassName());
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
            throw cRuntimeError("Can not serialize IGMP packet: type %d not supported.", pkt->getType());
    }
    igmp->igmp_cksum = TCPIPchecksum::checksum(igmp, b.getPos() - startPos);
}

cPacket *IGMPSerializer::parse(Buffer &b, Context& context)
{
    unsigned int startPos = b.getPos();
    struct igmp *igmp = (struct igmp *)b.accessNBytes(sizeof(struct igmp));
    if (!igmp) {
        b.seek(startPos);
        return nullptr;
    }

    cPacket *packet = nullptr;

    switch (igmp->igmp_type) {
        case IGMP_MEMBERSHIP_QUERY:
            if (igmp->igmp_code == 0)
            {
                IGMPv1Query *pkt;
                packet = pkt = new IGMPv1Query();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
            }
            else
            {
                IGMPv2Query *pkt;
                packet = pkt = new IGMPv2Query();
                pkt->setMaxRespTime(igmp->igmp_code);
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
                return pkt;
            }
            break;

        case IGMP_V1_MEMBERSHIP_REPORT:
            {
                IGMPv1Report *pkt;
                packet = pkt = new IGMPv1Report();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
            }
            break;

        case IGMP_V2_MEMBERSHIP_REPORT:
            {
                IGMPv2Report *pkt;
                packet = pkt = new IGMPv2Report();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
            }
            break;

        case IGMP_V2_LEAVE_GROUP:
            {
                IGMPv2Leave *pkt;
                packet = pkt = new IGMPv2Leave();
                pkt->setGroupAddress(IPv4Address(ntohl(igmp->igmp_group.s_addr)));
            }
            break;

        default:
            throw cRuntimeError("IGMPSerializer: can not create IGMP packet: type %d not supported", (int)igmp->igmp_type);
    }

    if (TCPIPchecksum::checksum(igmp, sizeof(struct igmp)) != 0)
        packet->setBitError(true);
    return packet;
}

} // namespace serializer

} // namespace inet

