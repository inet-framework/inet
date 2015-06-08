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

#include "inet/common/serializer/ipv6/ICMPv6Serializer.h"

#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/icmpv6/IPv6NDMessage_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

Register_Serializer(ICMPv6Message, IP_PROT, IP_PROT_IPv6_ICMP, ICMPv6Serializer);

void ICMPv6Serializer::serialize(const cPacket *_pkt, Buffer &b, Context& c)
{
    unsigned int startpos = b.getPos();
    const ICMPv6Message *pkt = check_and_cast<const ICMPv6Message *>(_pkt);

    switch (pkt->getType()) {
        case ICMPv6_ECHO_REQUEST: {
            const ICMPv6EchoRequestMsg *frame = check_and_cast<const ICMPv6EchoRequestMsg *>(pkt);
            PingPayload *pp = check_and_cast<PingPayload *>(pkt->getEncapsulatedPacket());
            b.writeByte(pkt->getType());
            b.writeByte(frame->getCode());
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

        case ICMPv6_ECHO_REPLY: {
            const ICMPv6EchoReplyMsg *frame = check_and_cast<const ICMPv6EchoReplyMsg *>(pkt);
            PingPayload *pp = check_and_cast<PingPayload *>(pkt->getEncapsulatedPacket());
            b.writeByte(pkt->getType());
            b.writeByte(frame->getCode());
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

        case ICMPv6_DESTINATION_UNREACHABLE: {
            const ICMPv6DestUnreachableMsg *frame = check_and_cast<const ICMPv6DestUnreachableMsg *>(pkt);
            b.writeByte(pkt->getType());
            b.writeByte(frame->getCode());
            b.writeUint16(0);   // crc
            b.writeUint32(0);   // unused
            SerializerBase::lookupAndSerialize(pkt->getEncapsulatedPacket(), b, c, ETHERTYPE, ETHERTYPE_IPv6);
            break;
        }

        case ICMPv6_TIME_EXCEEDED: {
            const ICMPv6TimeExceededMsg *frame = check_and_cast<const ICMPv6TimeExceededMsg *>(pkt);
            b.writeByte(pkt->getType());
            b.writeByte(frame->getCode());
            b.writeUint16(0);   // crc
            b.writeUint32(0);   // unused
            SerializerBase::lookupAndSerialize(pkt->getEncapsulatedPacket(), b, c, ETHERTYPE, ETHERTYPE_IPv6);
            break;
        }

        case ICMPv6_NEIGHBOUR_SOL: {
            const IPv6NeighbourSolicitation *frame = check_and_cast<const IPv6NeighbourSolicitation *>(pkt);
            b.writeByte(pkt->getType());
            b.writeByte(frame->getCode());
            b.writeUint16(0);   // crc
            b.writeUint32(0);   // unused
            b.writeIPv6Address(frame->getTargetAddress());
            if (frame->getByteLength() > 8 + 16) {   // has optional sourceLinkLayerAddress    (TLB options)
                b.writeByte(IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION);
                b.writeByte(1);         // length = 1 * 8byte
                b.writeMACAddress(frame->getSourceLinkLayerAddress());
                ASSERT(1 + 1 + MAC_ADDRESS_SIZE == 8);
            }
            break;
        }

        default: {
            if (c.throwOnSerializerNotFound)
                throw cRuntimeError("Can not serialize ICMPv6 packet: type %d  not supported.", pkt->getType());
            b.writeByte(pkt->getType());
            b.writeByte(0xFF);      // unknown
            b.writeUint16(0);   // crc
            ASSERT(pkt->getByteLength() >= 4);
            b.fillNBytes(pkt->getByteLength() - 4, 'a');
            break;
        }
    }
    b.writeUint16To(startpos + 2, TCPIPchecksum::checksum(b._getBuf() + startpos, b.getPos() - startpos));
}

cPacket *ICMPv6Serializer::deserialize(const Buffer &b, Context& context)
{
    ASSERT(b.getPos() == 0);

    ICMPv6Message *_pkt = nullptr;
    uint8_t type = b.readByte();     // type
    uint8_t subcode = b.readByte();  // subcode
    b.readUint16();   // crc

    switch (type) {
        case ICMPv6_ECHO_REQUEST: {
            ICMPv6EchoRequestMsg *pkt = new ICMPv6EchoRequestMsg(); _pkt = pkt;
            PingPayload *pp = new PingPayload();
            pkt->setType(type);
            pkt->setCode(subcode);
            pkt->setByteLength(4);
            pp->setOriginatorId(b.readUint16());
            uint16_t seqno = b.readUint16();
            pp->setSeqNo(seqno);

            char name[32];
            sprintf(name, "parsed-ping-%u", seqno);
            pp->setName(name);
            pkt->setName(name);

            pp->setByteLength(4 + b.getRemainingSize());
            pp->setDataArraySize(b.getRemainingSize());
            for (unsigned int i = 0; b.getRemainingSize() > 0; i++)
                pp->setData(i, b.readByte());
            pkt->encapsulate(pp);
            break;
        }

        case ICMPv6_ECHO_REPLY: {
            ICMPv6EchoReplyMsg *pkt = new ICMPv6EchoReplyMsg(); _pkt = pkt;
            PingPayload *pp = new PingPayload();
            pkt->setType(type);
            pkt->setCode(subcode);
            pkt->setByteLength(4);
            pp->setOriginatorId(b.readUint16());
            uint16_t seqno = b.readUint16();

            char name[32];
            sprintf(name, "parsed-ping-%u-reply", ntohs(seqno));
            pp->setName(name);
            pkt->setName(name);

            pp->setByteLength(4 + b.getRemainingSize());
            pp->setDataArraySize(b.getRemainingSize());
            for (unsigned int i = 0; b.getRemainingSize() > 0; i++)
                pp->setData(i, b.readByte());
            pkt->encapsulate(pp);
            break;
        }

        case ICMPv6_NEIGHBOUR_SOL: {    // RFC 4861 Section 4.3
            IPv6NeighbourSolicitation *pkt = new IPv6NeighbourSolicitation(); _pkt = pkt;
            pkt->setType(type);
            pkt->setCode(subcode);
            pkt->setByteLength(b._getBufSize());

            b.readUint32(); // reserved
            pkt->setTargetAddress(b.readIPv6Address());
            while (b.getRemainingSize()) {   // has options
                unsigned int pos = b.getPos();
                unsigned char type = b.readByte();
                unsigned char length = b.readByte();
                if (type == 0 || length == 0) {
                    pkt->setBitError(true);
                    break;
                }
                if (type == 1) {
                    pkt->setSourceLinkLayerAddress(b.readMACAddress());     // sourceLinkLayerAddress
                }
                b.seek(pos + 8 * length);
            }
            break;
        }

        default: {
            EV_ERROR << "Can not parse ICMP packet: type " << type << " not supported.";
            return nullptr;
        }
    }
    uint16_t ccrc = TCPIPchecksum::checksum(b._getBuf(), b.getPos());
    if (ccrc)
        _pkt->setBitError(true);
    return _pkt;
}

} // namespace serializer

} // namespace inet

