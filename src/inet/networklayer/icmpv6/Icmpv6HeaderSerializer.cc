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
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/icmpv6/Icmpv6HeaderSerializer.h"
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

Register_Serializer(Icmpv6Header, Icmpv6HeaderSerializer);
Register_Serializer(Icmpv6EchoRequestMsg, Icmpv6HeaderSerializer);
Register_Serializer(Icmpv6EchoReplyMsg, Icmpv6HeaderSerializer);
Register_Serializer(Icmpv6DestUnreachableMsg, Icmpv6HeaderSerializer);
Register_Serializer(Icmpv6PacketTooBigMsg, Icmpv6HeaderSerializer);
Register_Serializer(Icmpv6ParamProblemMsg, Icmpv6HeaderSerializer);
Register_Serializer(Icmpv6TimeExceededMsg, Icmpv6HeaderSerializer);
Register_Serializer(Ipv6NeighbourSolicitation, Icmpv6HeaderSerializer);
Register_Serializer(Ipv6NeighbourAdvertisement, Icmpv6HeaderSerializer);
Register_Serializer(Ipv6RouterSolicitation, Icmpv6HeaderSerializer);
Register_Serializer(Ipv6RouterAdvertisement, Icmpv6HeaderSerializer);

void Icmpv6HeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pkt = staticPtrCast<const Icmpv6Header>(chunk);

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
            stream.writeIpv6Address(frame->getTargetAddress());
            if (frame->getChunkLength() > B(8 + 16)) {   // has optional sourceLinkLayerAddress    (TLB options)
                stream.writeByte(IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION);
                stream.writeByte(1);         // length = 1 * 8byte
                stream.writeMacAddress(frame->getSourceLinkLayerAddress());
                ASSERT(1 + 1 + MAC_ADDRESS_SIZE == 8);
            }
            break;
        }

        case ICMPv6_NEIGHBOUR_AD: {
            auto frame = check_and_cast<const Ipv6NeighbourAdvertisement *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(0);   // unused
            //FIXME serialize TLV options
            // TODO: incomplete
            break;
        }

        case ICMPv6_ROUTER_SOL: {
            auto frame = check_and_cast<const Ipv6RouterSolicitation *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(0);   // unused
            //FIXME serialize TLV options
            // TODO: incomplete
            break;
        }

        case ICMPv6_ROUTER_AD: {
            auto frame = check_and_cast<const Ipv6RouterAdvertisement *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(0);   // unused
            //FIXME serialize TLV options
            // TODO: incomplete
            break;
        }

        default:
            throw cRuntimeError("Cannot serialize ICMPv6 packet: type %d  not supported.", pkt->getType());
    }
}

const Ptr<Chunk> Icmpv6HeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto icmpv6Header = makeShared<Icmpv6Header>();
    Icmpv6Type type = static_cast<Icmpv6Type>(stream.readByte());     // type
    uint8_t subcode = stream.readByte();  // subcode
    uint16_t chksum = stream.readUint16Be();

    switch (type) {
        case ICMPv6_ECHO_REQUEST: {
            auto echoRequest = makeShared<Icmpv6EchoRequestMsg>(); icmpv6Header = echoRequest;
            echoRequest->setType(type);
            echoRequest->setCode(subcode);
            break;
        }

        case ICMPv6_ECHO_REPLY: {
            auto echoReply = makeShared<Icmpv6EchoReplyMsg>(); icmpv6Header = echoReply;
            echoReply->setType(type);
            echoReply->setCode(subcode);
            break;
        }

        case ICMPv6_DESTINATION_UNREACHABLE: {
            auto destUnreach = makeShared<Icmpv6DestUnreachableMsg>(); icmpv6Header = destUnreach;
            destUnreach->setType(type);
            destUnreach->setCode(static_cast<Icmpv6DestUnav>(subcode));
            stream.readUint32Be();        // unused
            break;
        }

        case ICMPv6_TIME_EXCEEDED: {
            auto timeExceeded = makeShared<Icmpv6TimeExceededMsg>(); icmpv6Header = timeExceeded;
            timeExceeded->setType(type);
            timeExceeded->setCode(static_cast<Icmpv6TimeEx>(subcode));
            stream.readUint32Be();        // unused
            break;
        }

        case ICMPv6_NEIGHBOUR_SOL: {    // RFC 4861 Section 4.3
            auto neighbourSol = makeShared<Ipv6NeighbourSolicitation>(); icmpv6Header = neighbourSol;
            neighbourSol->setType(type);
            neighbourSol->setCode(subcode);

            stream.readUint32Be(); // reserved
            neighbourSol->setTargetAddress(stream.readIpv6Address());

            //FIXME deserialize TLV options
            while (stream.getRemainingLength() != B(0)) {   // has options
                unsigned char type = stream.readByte();
                unsigned char length = stream.readByte();
                if (type == 0 || length == 0) {
                    neighbourSol->markIncorrect();
                    break;
                }
                if (type == 1) {
                    neighbourSol->setSourceLinkLayerAddress(stream.readMacAddress());     // sourceLinkLayerAddress
                }
            }
            break;
        }

        case ICMPv6_NEIGHBOUR_AD: {
            auto neighbourAd = makeShared<Ipv6NeighbourAdvertisement>(); icmpv6Header = neighbourAd;
            neighbourAd->setType(type);
            neighbourAd->setCode(subcode);
            stream.readUint32Be(); // reserved
            //FIXME deserialize TLV options
            // TODO: incomplete
            break;
        }

        case ICMPv6_ROUTER_SOL: {
            auto routerSol = makeShared<Ipv6RouterSolicitation>(); icmpv6Header = routerSol;
            routerSol->setType(type);
            routerSol->setCode(subcode);
            stream.readUint32Be(); // reserved
            //FIXME deserialize TLV options
            // TODO: incomplete
            break;
        }

        case ICMPv6_ROUTER_AD: {
            auto routerAd = makeShared<Ipv6RouterAdvertisement>(); icmpv6Header = routerAd;
            routerAd->setType(type);
            routerAd->setCode(subcode);
            stream.readUint32Be(); // reserved
            //FIXME deserialize TLV options
            // TODO: incomplete
            break;
        }

        default: {
            EV_ERROR << "Can not parse ICMP packet: type " << (int)type << " not supported.";
            icmpv6Header->markImproperlyRepresented();
            break;
        }
    }
    icmpv6Header->setChksum(chksum);
    return icmpv6Header;
}

} // namespace inet

