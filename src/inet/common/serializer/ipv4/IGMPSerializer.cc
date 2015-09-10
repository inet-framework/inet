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

#include "inet/common/serializer/SerializerUtil.h"

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
    const void *igmp = b.accessNBytes(0);

    switch (pkt->getType())
    {
        case IGMP_MEMBERSHIP_QUERY: {
            b.writeByte(IGMP_MEMBERSHIP_QUERY);    // type
            b.writeByte(0);    // code
            b.writeUint16(0);    // chksum
            b.writeIPv4Address(check_and_cast<const IGMPQuery*>(pkt)->getGroupAddress());
            if (dynamic_cast<const IGMPv3Query*>(pkt))
            {
                const IGMPv3Query* v3pkt = static_cast<const IGMPv3Query*>(pkt);
                b.writeByteTo(1, v3pkt->getMaxRespCode());
                ASSERT(v3pkt->getRobustnessVariable() <= 7);
                b.writeByte((v3pkt->getSuppressRouterProc() ? 0x8 : 0) | v3pkt->getRobustnessVariable());
                b.writeByte(v3pkt->getQueryIntervalCode());
                unsigned int vs = v3pkt->getSourceList().size();
                b.writeUint16(vs);
                for (unsigned int i = 0; i < vs; i++)
                    b.writeIPv4Address(v3pkt->getSourceList()[i]);
            }
            else if (dynamic_cast<const IGMPv2Query*>(pkt))
            {
                b.writeByteTo(1, static_cast<const IGMPv2Query*>(pkt)->getMaxRespTime());
            }
            break;
        }

        case IGMPV1_MEMBERSHIP_REPORT:
            b.writeByte(IGMPV1_MEMBERSHIP_REPORT);    // type
            b.writeByte(0);    // code
            b.writeUint16(0);    // chksum
            b.writeIPv4Address(check_and_cast<const IGMPv1Report*>(pkt)->getGroupAddress());
            break;

        case IGMPV2_MEMBERSHIP_REPORT:
            b.writeByte(IGMPV2_MEMBERSHIP_REPORT);    // type
            b.writeByte(0);    // code
            b.writeUint16(0);    // chksum
            b.writeIPv4Address(check_and_cast<const IGMPv2Report*>(pkt)->getGroupAddress());
            break;

        case IGMPV2_LEAVE_GROUP:
            b.writeByte(IGMPV2_LEAVE_GROUP);    // type
            b.writeByte(0);    // code
            b.writeUint16(0);    // chksum
            b.writeIPv4Address(check_and_cast<const IGMPv2Leave*>(pkt)->getGroupAddress());
            break;

        case IGMPV3_MEMBERSHIP_REPORT: {
            const IGMPv3Report* v3pkt = check_and_cast<const IGMPv3Report*>(pkt);
            b.writeByte(IGMPV3_MEMBERSHIP_REPORT);    // type
            b.writeByte(0);    // code
            b.writeUint16(0);    // chksum
            b.writeUint16(0);    // reserved
            unsigned int s = v3pkt->getGroupRecordArraySize();
            b.writeUint16(s);    // number of groups
            for (unsigned int i = 0; i < s; i++) {
                // serialize one group:
                const GroupRecord& gr = v3pkt->getGroupRecord(i);
                b.writeByte(gr.recordType);
                b.writeByte(0);  // aux data len: RFC 3376 Section 4.2.6
                b.writeUint16(gr.sourceList.size());
                b.writeIPv4Address(gr.groupAddress);
                for (auto src: gr.sourceList) {
                    b.writeIPv4Address(src);
                }
                // write auxiliary data
            }
            break;
        }

        default:
            throw cRuntimeError("Can not serialize IGMP packet (%s): type %d not supported.", pkt->getClassName(), pkt->getType());
    }
    b.writeUint16To(2, TCPIPchecksum::checksum(igmp, b.getPos() - startPos));
}

cPacket *IGMPSerializer::deserialize(const Buffer &b, Context& c)
{
    unsigned int startPos = b.getPos();
    const void *igmp = b.accessNBytes(0);
    unsigned char type = b.readByte();
    unsigned char code = b.readByte();
    uint16_t chksum = b.readUint16(); (void)chksum;

    cPacket *packet = nullptr;

    switch (type) {
        case IGMP_MEMBERSHIP_QUERY:
            if (code == 0) {
                IGMPv1Query *pkt = new IGMPv1Query();
                packet = pkt;
                pkt->setGroupAddress(b.readIPv4Address());
                pkt->setByteLength(8);
            }
            else if (b._getBufSize() - startPos == 8) {        // RFC 3376 Section 7.1
                IGMPv2Query *pkt = new IGMPv2Query();
                packet = pkt;
                pkt->setMaxRespTime(code);
                pkt->setGroupAddress(b.readIPv4Address());
                pkt->setByteLength(8);
            }
            else {
                IGMPv3Query *pkt = new IGMPv3Query();
                packet = pkt;
                pkt->setMaxRespCode(code);
                pkt->setGroupAddress(b.readIPv4Address());
                unsigned char x = b.readByte(); //
                pkt->setSuppressRouterProc((x & 0x8) != 0);
                pkt->setRobustnessVariable(x & 7);
                pkt->setQueryIntervalCode(b.readByte());
                unsigned int vs = b.readUint16();
                for (unsigned int i = 0; i < vs && !b.hasError(); i++)
                    pkt->getSourceList()[i] = b.readIPv4Address();
                pkt->setByteLength(b.getPos() - startPos);
            }
            break;

        case IGMPV1_MEMBERSHIP_REPORT:
            {
                IGMPv1Report *pkt;
                packet = pkt = new IGMPv1Report();
                pkt->setGroupAddress(b.readIPv4Address());
                pkt->setByteLength(8);
            }
            break;

        case IGMPV2_MEMBERSHIP_REPORT:
            {
                IGMPv2Report *pkt;
                packet = pkt = new IGMPv2Report();
                pkt->setGroupAddress(b.readIPv4Address());
                pkt->setByteLength(8);
            }
            break;

        case IGMPV2_LEAVE_GROUP:
            {
                IGMPv2Leave *pkt;
                packet = pkt = new IGMPv2Leave();
                pkt->setGroupAddress(b.readIPv4Address());
                pkt->setByteLength(8);
            }
            break;

        case IGMPV3_MEMBERSHIP_REPORT:
            {
                IGMPv3Report *pkt;
                packet = pkt = new IGMPv3Report();
                b.readUint16(); //reserved
                unsigned int s = b.readUint16();
                pkt->setGroupRecordArraySize(s);
                unsigned int i;
                for (i = 0; i < s && !b.hasError(); i++) {
                    GroupRecord gr;
                    gr.recordType = b.readByte();
                    uint16_t auxDataLen = 4 * b.readByte();
                    unsigned int gac = b.readUint16();
                    gr.groupAddress = b.readIPv4Address();
                    for (unsigned int j = 0; j < gac && !b.hasError(); j++) {
                        gr.sourceList.push_back(b.readIPv4Address());
                    }
                    pkt->setGroupRecord(i, gr);
                    b.accessNBytes(auxDataLen);
                }
                if (i < s) {
                    pkt->setGroupRecordArraySize(i);
                }
                pkt->setByteLength(b.getPos() - startPos);
            }
            break;

        default:
            EV_ERROR << "IGMPSerializer: can not create IGMP packet: type " << type << " not supported\n";
            b.seek(startPos);
            packet = SerializerRegistrationList::byteArraySerializer.deserializePacket(b, c);
            packet->setBitError(true);
            break;
    }

    ASSERT(packet);
    if (b.hasError() || TCPIPchecksum::checksum(igmp, packet->getByteLength()) != 0)
        packet->setBitError(true);
    return packet;
}

} // namespace serializer

} // namespace inet

