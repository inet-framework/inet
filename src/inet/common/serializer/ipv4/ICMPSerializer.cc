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

#include "inet/common/serializer/ipv4/ICMPSerializer.h"

#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/ipv4/headers/ip.h"
#include "inet/common/serializer/ipv4/headers/ip_icmp.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/ipv4/ICMPMessage_m.h"

namespace inet {

namespace serializer {

Register_Serializer(ICMPMessage, IP_PROT, IP_PROT_ICMP, ICMPSerializer);

void ICMPSerializer::serialize(const cPacket *_pkt, Buffer &b, Context& c)
{
    unsigned int startpos = b.getPos();
    const ICMPMessage *pkt = check_and_cast<const ICMPMessage *>(_pkt);

    switch (pkt->getType()) {
        case ICMP_ECHO_REQUEST: {
            PingPayload *pp = check_and_cast<PingPayload *>(pkt->getEncapsulatedPacket());
            b.writeByte(ICMP_ECHO_REQUEST);
            b.writeByte(pkt->getCode());
            b.writeUint16(0);   // checksum
            b.writeUint16(pp->getOriginatorId());
            b.writeUint16(pp->getSeqNo());
            unsigned int datalen = pp->getDataArraySize();
            for (unsigned int i = 0; i < datalen; i++)
                b.writeByte(pp->getData(i));
            datalen = (pp->getByteLength() - 4) - datalen;
            b.fillNBytes(datalen, 'a');
            break;
        }

        case ICMP_ECHO_REPLY: {
            PingPayload *pp = check_and_cast<PingPayload *>(pkt->getEncapsulatedPacket());
            b.writeByte(ICMP_ECHO_REPLY);
            b.writeByte(pkt->getCode());
            b.writeUint16(0);   // checksum
            b.writeUint16(pp->getOriginatorId());
            b.writeUint16(pp->getSeqNo());
            unsigned int datalen = pp->getDataArraySize();
            for (unsigned int i = 0; i < datalen; i++)
                b.writeByte(pp->getData(i));
            datalen = (pp->getByteLength() - 4) - datalen;
            b.fillNBytes(datalen, 'a');
            break;
        }

        case ICMP_DESTINATION_UNREACHABLE: {
            b.writeByte(ICMP_DESTINATION_UNREACHABLE);
            b.writeByte(pkt->getCode());
            b.writeUint16(0);   // checksum
            b.writeUint16(0);   // unused
            b.writeUint16(0);   // next hop MTU
            Buffer s(b, b.getRemainingSize());    // save buffer error bit (encapsulated packet usually larger than ICMPPacket payload size)
            SerializerBase::lookupAndSerialize(pkt->getEncapsulatedPacket(), s, c, ETHERTYPE, ETHERTYPE_IPv4);
            b.accessNBytes(std::min((unsigned int)(pkt->getByteLength() - ICMP_MINLEN), s.getPos()));
            break;
        }

        case ICMP_TIME_EXCEEDED: {
            b.writeByte(ICMP_TIME_EXCEEDED);
            b.writeByte(ICMP_TIMXCEED_INTRANS);
            b.writeUint16(0);   // checksum
            b.writeUint32(0);   // unused
            Buffer s(b, b.getRemainingSize());    // save buffer error bit (encapsulated packet usually larger than ICMPPacket payload size)
            SerializerBase::lookupAndSerialize(pkt->getEncapsulatedPacket(), s, c, ETHERTYPE, ETHERTYPE_IPv4);
            b.accessNBytes(std::min((unsigned int)(pkt->getByteLength() - ICMP_MINLEN), s.getPos()));
            break;
        }

        default: {
            //TODO if (c.throwOnSerializerNotFound)
            throw cRuntimeError("Can not serialize ICMP packet: type %d  not supported.", pkt->getType());
        }
    }
    b.writeUint16To(startpos + 2, TCPIPchecksum::checksum(b._getBuf() + startpos, b.getPos() - startpos));
}

cPacket *ICMPSerializer::deserialize(const Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    ICMPMessage *pkt = new ICMPMessage("parsed-icmp");
    uint8_t type = b.readByte();     // type
    uint8_t subcode = b.readByte();  // subcode
    b.readUint16();   // checksum

    switch (type) {
        case ICMP_ECHO_REQUEST: {
            PingPayload *pp = new PingPayload();
            pkt->setType(ICMP_ECHO_REQUEST);
            pkt->setCode(subcode);
            pkt->setByteLength(4);
            pp->setOriginatorId(b.readUint16());
            uint16_t seqno = b.readUint16();
            pp->setSeqNo(seqno);

            char name[32];
            sprintf(name, "parsed-ping%d", seqno);
            pp->setName(name);
            pkt->setName(name);

            pp->setByteLength(4 + b.getRemainingSize());
            pp->setDataArraySize(b.getRemainingSize());
            for (unsigned int i = 0; b.getRemainingSize() > 0; i++)
                pp->setData(i, b.readByte());
            pkt->encapsulate(pp);
            break;
        }

        case ICMP_ECHO_REPLY: {
            PingPayload *pp = new PingPayload();
            pkt->setType(ICMP_ECHO_REPLY);
            pkt->setCode(subcode);
            pkt->setByteLength(4);
            pp->setOriginatorId(b.readUint16());
            uint16_t seqno = b.readUint16();
            pp->setSeqNo(seqno);

            char name[32];
            sprintf(name, "parsed-ping%d-reply", seqno);
            pp->setName(name);
            pkt->setName(name);

            pp->setByteLength(4 + b.getRemainingSize());
            pp->setDataArraySize(b.getRemainingSize());
            for (unsigned int i = 0; b.getRemainingSize() > 0; i++)
                pp->setData(i, b.readByte());
            pkt->encapsulate(pp);
            break;
        }

        case ICMP_DESTINATION_UNREACHABLE: {
            pkt->setType(ICMP_DESTINATION_UNREACHABLE);
            pkt->setCode(subcode);
            b.readUint16();   // unused
            b.readUint16();   // next hop MTU
            pkt->setByteLength(8);
            Buffer s(b, b.getRemainingSize());    // save buffer error bit (encapsulated packet usually larger than ICMPPacket payload size)
            cPacket *pp = SerializerBase::lookupAndDeserialize(s, c, ETHERTYPE, ETHERTYPE_IPv4);
            b.accessNBytes(s.getPos());
            pkt->encapsulate(pp);
            pkt->setByteLength(b.getPos());
            break;
        }

        case ICMP_TIME_EXCEEDED: {
            pkt->setType(ICMP_TIME_EXCEEDED);
            pkt->setCode(subcode);
            b.readUint32();   // unused
            pkt->setByteLength(8);
            Buffer s(b, b.getRemainingSize());    // save buffer error bit (encapsulated packet usually larger than ICMPPacket payload size)
            cPacket *pp = SerializerBase::lookupAndDeserialize(s, c, ETHERTYPE, ETHERTYPE_IPv4);
            b.accessNBytes(s.getPos());
            pkt->encapsulate(pp);
            pkt->setByteLength(b.getPos());
            break;
        }

        default: {
            EV_ERROR << "Can not parse ICMP packet: type " << type << " not supported.";
            delete pkt;
            return nullptr;
        }
    }
    uint16_t cchecksum = TCPIPchecksum::checksum(b._getBuf(), b.getPos());
    if (cchecksum)
        pkt->setBitError(true);
    return pkt;
}

} // namespace serializer

} // namespace inet

