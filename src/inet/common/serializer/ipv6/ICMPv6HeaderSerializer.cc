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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/serializer/ipv6/ICMPv6HeaderSerializer.h"
#include "inet/networklayer/icmpv6/ICMPv6Header_m.h"
#include "inet/networklayer/icmpv6/IPv6NDMessage_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

Register_Serializer(Icmpv6Header, ICMPv6HeaderSerializer);
Register_Serializer(Icmpv6EchoRequestMsg, ICMPv6HeaderSerializer);
Register_Serializer(Icmpv6EchoReplyMsg, ICMPv6HeaderSerializer);
Register_Serializer(Icmpv6DestUnreachableMsg, ICMPv6HeaderSerializer);
Register_Serializer(Icmpv6PacketTooBigMsg, ICMPv6HeaderSerializer);
Register_Serializer(Icmpv6ParamProblemMsg, ICMPv6HeaderSerializer);
Register_Serializer(Icmpv6TimeExceededMsg, ICMPv6HeaderSerializer);
Register_Serializer(Ipv6NeighbourSolicitation, ICMPv6HeaderSerializer);

void ICMPv6HeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pkt = std::static_pointer_cast<const Icmpv6Header>(chunk);

    switch (pkt->getType()) {
        case ICMPv6_ECHO_REQUEST: {
            const Icmpv6EchoRequestMsg *frame = check_and_cast<const Icmpv6EchoRequestMsg *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            break;
        }

        case ICMPv6_ECHO_REPLY: {
            const Icmpv6EchoReplyMsg *frame = check_and_cast<const Icmpv6EchoReplyMsg *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            break;
        }

        case ICMPv6_DESTINATION_UNREACHABLE: {
            auto frame = check_and_cast<const Icmpv6DestUnreachableMsg *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(0);   // unused
            break;
        }

        case ICMPv6_TIME_EXCEEDED: {
            auto frame = check_and_cast<const Icmpv6TimeExceededMsg *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(0);   // unused
            break;
        }

        case ICMPv6_NEIGHBOUR_SOL: {
            auto frame = check_and_cast<const Ipv6NeighbourSolicitation *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(0);   // unused
            stream.writeIPv6Address(frame->getTargetAddress());
            if (frame->getChunkLength() > byte(8 + 16)) {   // has optional sourceLinkLayerAddress    (TLB options)
                stream.writeByte(IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION);
                stream.writeByte(1);         // length = 1 * 8byte
                stream.writeMACAddress(frame->getSourceLinkLayerAddress());
                ASSERT(1 + 1 + MAC_ADDRESS_SIZE == 8);
            }
            break;
        }

        default:
            throw cRuntimeError("Cannot serialize ICMPv6 packet: type %d  not supported.", pkt->getType());
    }
}

const Ptr<Chunk> ICMPv6HeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Icmpv6Header> _pkt = nullptr;
    uint8_t type = stream.readByte();     // type
    uint8_t subcode = stream.readByte();  // subcode
    uint16_t chksum = stream.readUint16Be();

    switch (type) {
        case ICMPv6_ECHO_REQUEST: {
            auto pkt = std::make_shared<Icmpv6EchoRequestMsg>(); _pkt = pkt;
            pkt->setType(type);
            pkt->setCode(subcode);
            break;
        }

        case ICMPv6_ECHO_REPLY: {
            auto pkt = std::make_shared<Icmpv6EchoReplyMsg>(); _pkt = pkt;
            pkt->setType(type);
            pkt->setCode(subcode);
            break;
        }

        case ICMPv6_DESTINATION_UNREACHABLE: {
            auto pkt = std::make_shared<Icmpv6DestUnreachableMsg>(); _pkt = pkt;
            pkt->setType(type);
            pkt->setCode(subcode);
            stream.readUint32Be();        // unused
            break;
        }

        case ICMPv6_TIME_EXCEEDED: {
            auto pkt = std::make_shared<Icmpv6TimeExceededMsg>(); _pkt = pkt;
            pkt->setType(type);
            pkt->setCode(subcode);
            stream.readUint32Be();        // unused
            break;
        }

        case ICMPv6_NEIGHBOUR_SOL: {    // RFC 4861 Section 4.3
            auto pkt = std::make_shared<Ipv6NeighbourSolicitation>(); _pkt = pkt;
            pkt->setType(type);
            pkt->setCode(subcode);

            stream.readUint32Be(); // reserved
            pkt->setTargetAddress(stream.readIPv6Address());
            while (stream.getRemainingLength() != byte(0)) {   // has options
                unsigned char type = stream.readByte();
                unsigned char length = stream.readByte();
                if (type == 0 || length == 0) {
                    pkt->markIncorrect();
                    break;
                }
                if (type == 1) {
                    pkt->setSourceLinkLayerAddress(stream.readMACAddress());     // sourceLinkLayerAddress
                }
            }
            break;
        }

        default: {
            EV_ERROR << "Can not parse ICMP packet: type " << type << " not supported.";
            return nullptr;
        }
    }
    _pkt->setChksum(chksum);
    return _pkt;
}

} // namespace serializer

} // namespace inet

