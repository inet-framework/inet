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

void serializeIpv6NdOptions(MemoryOutputStream& stream, const Ipv6NdOptions& options)
{
    for (size_t i=0; i < options.getOptionArraySize(); i++) {
        const Ipv6NdOption *option = options.getOption(i);
        stream.writeByte(option->getType());
        stream.writeByte(option->getOptionLength());
        switch (option->getType()) {
        case IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION:
        case IPv6ND_TARGET_LINK_LAYER_ADDR_OPTION: {
            stream.writeMacAddress(check_and_cast<const Ipv6NdSourceTargetLinkLayerAddress*>(option)->getLinkLayerAddress());
            break;
        }
        case IPv6ND_PREFIX_INFORMATION: {
            auto pi = check_and_cast<const Ipv6NdPrefixInformation*>(option);
            stream.writeByte(pi->getPrefixLength());
            uint8_t res1 = (pi->getReserved1() & 0x1fu)
                    | (pi->getOnlinkFlag() ? 0x80u:0)
                    | (pi->getAutoAddressConfFlag() ? 0x40u:0)
                    | (pi->getRouterAddressFlag() ? 0x20u:0);
            stream.writeByte(res1);
            stream.writeUint32Be(pi->getValidLifetime());
            stream.writeUint32Be(pi->getPreferredLifetime());
            stream.writeUint32Be(pi->getReserved2());
            stream.writeIpv6Address(pi->getPrefix());
            break;
        }
        case IPv6ND_MTU: {
            auto mtu = check_and_cast<const Ipv6NdMtu*>(option);
            stream.writeUint16Be(mtu->getReserved());
            stream.writeUint32Be(mtu->getMtu());
            break;
        }
        case IPv6ND_ADVERTISEMENT_INTERVAL: {
            auto advInt = check_and_cast<const Mipv6NdAdvertisementInterval*>(option);
            stream.writeUint16Be(advInt->getReserved());
            stream.writeUint32Be(advInt->getAdvertisementInterval());
            break;
        }
        case IPv6ND_HOME_AGENT_INFORMATION_OPTION: {
            auto haInf = check_and_cast<const Mipv6HaInformation*>(option);
            stream.writeUint16Be(haInf->getReserved());
            stream.writeUint16Be(haInf->getHomeAgentPreference());
            stream.writeUint16Be(haInf->getHomeAgentLifetime());
            break;
        }
        default:
            throw cRuntimeError("Unknown IPv6ND option type=%i", option->getType());
        }
        for (size_t j=0; j < option->getPaddingBytesArraySize(); j++)
            stream.writeByte(option->getPaddingBytes(j));
    }
}

void deserializeIpv6NdOptions(Ipv6NdMessage& msg, Ipv6NdOptions& options, MemoryInputStream& stream)
{
    // deserialize TLV options
    while (stream.getRemainingLength() != B(0)) {   // has options
        unsigned char type = stream.readByte();
        unsigned char length = stream.readByte();
        if (length == 0) {
            msg.markIncorrect();
            break;
        }
        switch (type) {
            case IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION: {
                auto option = new Ipv6NdSourceLinkLayerAddress();
                option->setLinkLayerAddress(stream.readMacAddress());
                if (length > 1) {
                    option->setPaddingBytesArraySize(8 * (length-1));
                    for (int i = 0; i < 8 * (length-1); i++)
                        option->setPaddingBytes(i, stream.readByte());
                }
                options.insertOption(option);
                break;
            }
            case IPv6ND_TARGET_LINK_LAYER_ADDR_OPTION: {
                auto option = new Ipv6NdTargetLinkLayerAddress();
                option->setLinkLayerAddress(stream.readMacAddress());
                if (length > 1) {
                    option->setOptionLength(length);
                    option->setPaddingBytesArraySize(8 * (length-1));
                    for (int i = 0; i < 8 * (length-1); i++)
                        option->setPaddingBytes(i, stream.readByte());
                }
                options.insertOption(option);
                break;
            }
            case IPv6ND_PREFIX_INFORMATION: {
                auto option = new Ipv6NdPrefixInformation();
                option->setPrefixLength(stream.readByte());
                uint8_t reserved1 = stream.readByte();
                option->setOnlinkFlag((reserved1 & 0x80u) != 0);
                option->setAutoAddressConfFlag((reserved1 & 0x40u) != 0);
                option->setRouterAddressFlag((reserved1 & 0x20u) != 0);
                option->setReserved1(reserved1 & 0x1fu);
                option->setValidLifetime(stream.readUint32Be());
                option->setPreferredLifetime(stream.readUint32Be());
                option->setReserved2(stream.readUint32Be());
                option->setPrefix(stream.readIpv6Address());
                if (length > 4) {
                    option->setOptionLength(length);
                    option->setPaddingBytesArraySize(8 * (length-4));
                    for (int i = 0; i < 8 * (length-4); i++)
                        option->setPaddingBytes(i, stream.readByte());
                }
                options.insertOption(option);
                break;
            }
//            case IPv6ND_REDIRECTED_HEADER: {
//                break;
//            }
            case IPv6ND_MTU: {
                auto option = new Ipv6NdMtu();
                option->setReserved(stream.readUint16Be());
                option->setMtu(stream.readUint32Be());
                if (length > 1) {
                    option->setOptionLength(length);
                    option->setPaddingBytesArraySize(8 * (length-1));
                    for (int i = 0; i < 8 * (length-1); i++)
                        option->setPaddingBytes(i, stream.readByte());
                }
                options.insertOption(option);
                break;
            }
            case IPv6ND_ADVERTISEMENT_INTERVAL: {
                auto option = new Mipv6NdAdvertisementInterval();
                option->setReserved(stream.readUint16Be());
                option->setAdvertisementInterval(stream.readUint32Be());
                if (length > 1) {
                    option->setOptionLength(length);
                    option->setPaddingBytesArraySize(8 * (length-1));
                    for (int i = 0; i < 8 * (length-1); i++)
                        option->setPaddingBytes(i, stream.readByte());
                }
                options.insertOption(option);
                break;
            }
            case IPv6ND_HOME_AGENT_INFORMATION_OPTION: {
                auto option = new Mipv6HaInformation();
                option->setReserved(stream.readUint16Be());
                option->setHomeAgentPreference(stream.readUint16Be());
                option->setHomeAgentLifetime(stream.readUint16Be());
                if (length > 1) {
                    option->setOptionLength(length);
                    option->setPaddingBytesArraySize(8 * (length-1));
                    for (int i = 0; i < 8 * (length-1); i++)
                        option->setPaddingBytes(i, stream.readByte());
                }
                options.insertOption(option);
                break;
            }
            default: {
                auto option = new Ipv6NdOption();
                option->setType(static_cast<Ipv6NdOptionTypes>(type));
                option->setOptionLength(length);
                option->setPaddingBytesArraySize(8 * length - 2);
                for (int i = 0; i < 8 * length - 2; i++)
                    option->setPaddingBytes(i, stream.readByte());
                options.insertOption(option);
            }
        }
    }
}

void Icmpv6HeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pkt = staticPtrCast<const Icmpv6Header>(chunk);

    switch (pkt->getType()) {
        case ICMPv6_ECHO_REQUEST: {
            const Icmpv6EchoRequestMsg *frame = check_and_cast<const Icmpv6EchoRequestMsg *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint16Be(frame->getIdentifier());
            stream.writeUint16Be(frame->getSeqNumber());
            break;
        }

        case ICMPv6_ECHO_REPLY: {
            const Icmpv6EchoReplyMsg *frame = check_and_cast<const Icmpv6EchoReplyMsg *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint16Be(frame->getIdentifier());
            stream.writeUint16Be(frame->getSeqNumber());
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
            stream.writeUint32Be(frame->getReserved());
            stream.writeIpv6Address(frame->getTargetAddress());
            serializeIpv6NdOptions(stream, frame->getOptions());
            break;
        }

        case ICMPv6_NEIGHBOUR_AD: {
            auto frame = check_and_cast<const Ipv6NeighbourAdvertisement *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(
                      (frame->getRouterFlag() ? 0x80000000u : 0)
                    | (frame->getSolicitedFlag() ? 0x40000000u : 0)
                    | (frame->getOverrideFlag() ? 0x20000000u : 0)
                    | (frame->getReserved() & 0x1fffffffu)
                    );
            stream.writeIpv6Address(frame->getTargetAddress());
            serializeIpv6NdOptions(stream, frame->getOptions());
            break;
        }

        case ICMPv6_ROUTER_SOL: {
            auto frame = check_and_cast<const Ipv6RouterSolicitation *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint32Be(frame->getReserved());   // unused
            serializeIpv6NdOptions(stream, frame->getOptions());
            break;
        }

        case ICMPv6_ROUTER_AD: {
            auto frame = check_and_cast<const Ipv6RouterAdvertisement *>(pkt.get());
            stream.writeByte(pkt->getType());
            stream.writeByte(frame->getCode());
            stream.writeUint16Be(frame->getChksum());
            stream.writeUint8(frame->getCurHopLimit());
            stream.writeUint8(
                      (frame->getManagedAddrConfFlag() ? 0x80u : 0)
                    | (frame->getOtherStatefulConfFlag() ? 0x40u : 0)
                    | (frame->getHomeAgentFlag() ? 0x20u : 0)
                    | (frame->getReserved() & 0x1fu)
                    );
            stream.writeUint16Be(frame->getRouterLifetime());
            stream.writeUint32Be(frame->getReachableTime());
            stream.writeUint32Be(frame->getRetransTimer());
            serializeIpv6NdOptions(stream, frame->getOptions());
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
            echoRequest->setIdentifier(stream.readUint16Be());
            echoRequest->setSeqNumber(stream.readUint16Be());
            break;
        }

        case ICMPv6_ECHO_REPLY: {
            auto echoReply = makeShared<Icmpv6EchoReplyMsg>(); icmpv6Header = echoReply;
            echoReply->setType(type);
            echoReply->setCode(subcode);
            echoReply->setIdentifier(stream.readUint16Be());
            echoReply->setSeqNumber(stream.readUint16Be());
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
            neighbourSol->setReserved(stream.readUint32Be());
            neighbourSol->setTargetAddress(stream.readIpv6Address());
            deserializeIpv6NdOptions(*neighbourSol, neighbourSol->getOptionsForUpdate(), stream);
            break;
        }

        case ICMPv6_NEIGHBOUR_AD: {
            auto neighbourAd = makeShared<Ipv6NeighbourAdvertisement>(); icmpv6Header = neighbourAd;
            neighbourAd->setType(type);
            neighbourAd->setCode(subcode);
            uint32_t reserved = stream.readUint32Be(); // reserved
            neighbourAd->setRouterFlag((reserved & 0x80000000u) != 0);
            neighbourAd->setSolicitedFlag((reserved & 0x40000000u) != 0);
            neighbourAd->setOverrideFlag((reserved & 0x20000000u) != 0);
            neighbourAd->setReserved(reserved & 0x1fffffffu);
            neighbourAd->setTargetAddress(stream.readIpv6Address());
            deserializeIpv6NdOptions(*neighbourAd, neighbourAd->getOptionsForUpdate(), stream);
            break;
        }

        case ICMPv6_ROUTER_SOL: {
            auto routerSol = makeShared<Ipv6RouterSolicitation>(); icmpv6Header = routerSol;
            routerSol->setType(type);
            routerSol->setCode(subcode);
            routerSol->setReserved(stream.readUint32Be());
            deserializeIpv6NdOptions(*routerSol, routerSol->getOptionsForUpdate(), stream);
            break;
        }

        case ICMPv6_ROUTER_AD: {
            auto routerAd = makeShared<Ipv6RouterAdvertisement>(); icmpv6Header = routerAd;
            routerAd->setType(type);
            routerAd->setCode(subcode);
            routerAd->setCurHopLimit(stream.readUint8());
            uint32_t reserved = stream.readUint8(); // reserved
            routerAd->setManagedAddrConfFlag((reserved & 0x80u) != 0);
            routerAd->setOtherStatefulConfFlag((reserved & 0x40u) != 0);
            routerAd->setHomeAgentFlag((reserved & 0x20u) != 0);
            routerAd->setReserved(reserved & 0x1fu);
            routerAd->setRouterLifetime(stream.readUint16Be());
            routerAd->setReachableTime(stream.readUint32Be());
            routerAd->setRetransTimer(stream.readUint32Be());
            deserializeIpv6NdOptions(*routerAd, routerAd->getOptionsForUpdate(), stream);
            break;
        }

        default: {
            EV_ERROR << "Can not parse ICMP packet: type " << (int)type << " not supported.";
            icmpv6Header->markImproperlyRepresented();
            break;
        }
    }
    icmpv6Header->setCrcMode(CRC_COMPUTED);
    icmpv6Header->setChksum(chksum);
    return icmpv6Header;
}

} // namespace inet

