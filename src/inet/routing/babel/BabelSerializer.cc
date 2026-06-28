//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/routing/babel/BabelDefs.h"

namespace inet {
namespace babel {

Register_Serializer(BabelHeader, BabelHeaderSerializer);
Register_Serializer(BabelTlv, BabelTlvSerializer);
Register_Serializer(BabelPad1Tlv, BabelTlvSerializer);
Register_Serializer(BabelPadNTlv, BabelTlvSerializer);
Register_Serializer(BabelAckReqTlv, BabelTlvSerializer);
Register_Serializer(BabelAckTlv, BabelTlvSerializer);
Register_Serializer(BabelHelloTlv, BabelTlvSerializer);
Register_Serializer(BabelIhuTlv, BabelTlvSerializer);
Register_Serializer(BabelRouterIdTlv, BabelTlvSerializer);
Register_Serializer(BabelNextHopTlv, BabelTlvSerializer);
Register_Serializer(BabelUpdateTlv, BabelTlvSerializer);
Register_Serializer(BabelRouteReqTlv, BabelTlvSerializer);
Register_Serializer(BabelSeqnoReqTlv, BabelTlvSerializer);

// ---- AE address codec (RFC 6126, 4.1.1) ----

// Writes a full address per its address-encoding: IPv4 (4 bytes), IPv6 (16),
// or link-local IPv6 (8, the fe80::/64 prefix being implied). WILDCARD writes
// nothing.
static void writeAddress(MemoryOutputStream& stream, int ae, const L3Address& addr)
{
    switch (ae) {
        case AE::WILDCARD:
            break;
        case AE::IPv4:
            stream.writeIpv4Address(addr.toIpv4());
            break;
        case AE::IPv6:
            stream.writeIpv6Address(addr.toIpv6());
            break;
        case AE::LLIPv6: {
            const uint32_t *w = addr.toIpv6().words();
            stream.writeUint32Be(w[2]);
            stream.writeUint32Be(w[3]);
            break;
        }
        default:
            throw cRuntimeError("BabelSerializer: cannot serialize address with AE %d", ae);
    }
}

static L3Address readAddress(MemoryInputStream& stream, int ae)
{
    switch (ae) {
        case AE::WILDCARD:
            return L3Address();
        case AE::IPv4:
            return L3Address(stream.readIpv4Address());
        case AE::IPv6:
            return L3Address(stream.readIpv6Address());
        case AE::LLIPv6: {
            uint32_t w2 = stream.readUint32Be();
            uint32_t w3 = stream.readUint32Be();
            return L3Address(Ipv6Address(LINK_LOCAL_PREFIX, 0, w2, w3));
        }
        default:
            throw cRuntimeError("BabelSerializer: cannot deserialize address with AE %d", ae);
    }
}

// Writes the significant bytes of a (masked) prefix; ceil(plen/8) bytes in
// network order. This port never omits leading bytes (omitted = 0).
static void writePrefix(MemoryOutputStream& stream, int ae, const L3Address& prefix, uint8_t plen)
{
    int nbytes = bitsToBytesLen(plen);
    if (ae == AE::IPv4) {
        Ipv4Address a = prefix.toIpv4();
        for (int i = 0; i < nbytes; i++)
            stream.writeByte(static_cast<uint8_t>(a.getDByte(i)));
    }
    else if (ae == AE::IPv6) {
        const uint32_t *w = prefix.toIpv6().words();
        for (int i = 0; i < nbytes; i++)
            stream.writeByte(static_cast<uint8_t>((w[i / 4] >> (24 - 8 * (i % 4))) & 0xFF));
    }
    // AE WILDCARD: plen is 0, nbytes is 0 -> nothing to write
}

static L3Address readPrefix(MemoryInputStream& stream, int ae, uint8_t plen)
{
    int nbytes = bitsToBytesLen(plen);
    if (ae == AE::IPv4) {
        uint32_t p = 0;
        for (int i = 0; i < nbytes; i++)
            p |= static_cast<uint32_t>(stream.readByte()) << (24 - 8 * i);
        return L3Address(Ipv4Address(p).doAnd(Ipv4Address::makeNetmask(plen)));
    }
    else if (ae == AE::IPv6) {
        uint32_t w[4] = { 0, 0, 0, 0 };
        for (int i = 0; i < nbytes; i++)
            w[i / 4] |= static_cast<uint32_t>(stream.readByte()) << (24 - 8 * (i % 4));
        return L3Address(Ipv6Address().setPrefix(Ipv6Address(w[0], w[1], w[2], w[3]), plen));
    }
    return L3Address(); // wildcard
}

// ---- wire length ----

B babelTlvLength(const BabelTlv *tlv)
{
    switch (tlv->getTlvType()) {
        case BABEL_PAD1:
            return B(1);
        case BABEL_PADN:
            return B(2) + B(check_and_cast<const BabelPadNTlv *>(tlv)->getN());
        case BABEL_ACKREQ:
            return B(8);
        case BABEL_ACK:
            return B(4);
        case BABEL_HELLO:
            return B(8);
        case BABEL_IHU:
            return B(8) + B(AE::maxLen(check_and_cast<const BabelIhuTlv *>(tlv)->getAe()));
        case BABEL_ROUTERID:
            return B(12);
        case BABEL_NEXTHOP:
            return B(4) + B(AE::maxLen(check_and_cast<const BabelNextHopTlv *>(tlv)->getAe()));
        case BABEL_UPDATE:
            return B(12) + B(bitsToBytesLen(check_and_cast<const BabelUpdateTlv *>(tlv)->getPrefixLen()));
        case BABEL_ROUTEREQ:
            return B(4) + B(bitsToBytesLen(check_and_cast<const BabelRouteReqTlv *>(tlv)->getPrefixLen()));
        case BABEL_SEQNOREQ:
            return B(16) + B(bitsToBytesLen(check_and_cast<const BabelSeqnoReqTlv *>(tlv)->getPrefixLen()));
        default:
            throw cRuntimeError("BabelSerializer: cannot compute length of unknown TLV type %d", tlv->getTlvType());
    }
}

// ---- header ----

void BabelHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const BabelHeader>(chunk);
    stream.writeUint8(header->getMagic());
    stream.writeUint8(header->getVersion());
    stream.writeUint16Be(header->getBodyLength());
}

const Ptr<Chunk> BabelHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<BabelHeader>();
    header->setMagic(stream.readUint8());
    header->setVersion(stream.readUint8());
    header->setBodyLength(stream.readUint16Be());
    return header;
}

// ---- TLVs ----

void BabelTlvSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& tlv = staticPtrCast<const BabelTlv>(chunk);
    uint8_t type = tlv->getTlvType();

    stream.writeUint8(type);
    if (type == BABEL_PAD1)
        return; // Pad1 has no length octet and no body

    // length octet: number of body bytes after the length octet
    stream.writeUint8(static_cast<uint8_t>(babelTlvLength(tlv.get()).get<B>() - 2));

    switch (type) {
        case BABEL_PADN: {
            const auto& t = staticPtrCast<const BabelPadNTlv>(chunk);
            for (uint8_t i = 0; i < t->getN(); i++)
                stream.writeByte(0);
            break;
        }
        case BABEL_ACKREQ: {
            const auto& t = staticPtrCast<const BabelAckReqTlv>(chunk);
            stream.writeUint16Be(t->getReserved());
            stream.writeUint16Be(t->getNonce());
            stream.writeUint16Be(t->getInterval());
            break;
        }
        case BABEL_ACK: {
            const auto& t = staticPtrCast<const BabelAckTlv>(chunk);
            stream.writeUint16Be(t->getNonce());
            break;
        }
        case BABEL_HELLO: {
            const auto& t = staticPtrCast<const BabelHelloTlv>(chunk);
            stream.writeUint16Be(t->getReserved());
            stream.writeUint16Be(t->getSeqno());
            stream.writeUint16Be(t->getInterval());
            break;
        }
        case BABEL_IHU: {
            const auto& t = staticPtrCast<const BabelIhuTlv>(chunk);
            stream.writeUint8(t->getAe());
            stream.writeUint8(t->getReserved());
            stream.writeUint16Be(t->getRxcost());
            stream.writeUint16Be(t->getInterval());
            writeAddress(stream, t->getAe(), t->getAddress());
            break;
        }
        case BABEL_ROUTERID: {
            const auto& t = staticPtrCast<const BabelRouterIdTlv>(chunk);
            stream.writeUint16Be(t->getReserved());
            stream.writeUint32Be(t->getRouterIdHi());
            stream.writeUint32Be(t->getRouterIdLo());
            break;
        }
        case BABEL_NEXTHOP: {
            const auto& t = staticPtrCast<const BabelNextHopTlv>(chunk);
            stream.writeUint8(t->getAe());
            stream.writeUint8(t->getReserved());
            writeAddress(stream, t->getAe(), t->getNextHop());
            break;
        }
        case BABEL_UPDATE: {
            const auto& t = staticPtrCast<const BabelUpdateTlv>(chunk);
            stream.writeUint8(t->getAe());
            stream.writeUint8(t->getFlags());
            stream.writeUint8(t->getPrefixLen());
            stream.writeUint8(0); // omitted: this port never compresses
            stream.writeUint16Be(t->getInterval());
            stream.writeUint16Be(t->getSeqno());
            stream.writeUint16Be(t->getMetric());
            writePrefix(stream, t->getAe(), t->getPrefix(), t->getPrefixLen());
            break;
        }
        case BABEL_ROUTEREQ: {
            const auto& t = staticPtrCast<const BabelRouteReqTlv>(chunk);
            stream.writeUint8(t->getAe());
            stream.writeUint8(t->getPrefixLen());
            writePrefix(stream, t->getAe(), t->getPrefix(), t->getPrefixLen());
            break;
        }
        case BABEL_SEQNOREQ: {
            const auto& t = staticPtrCast<const BabelSeqnoReqTlv>(chunk);
            stream.writeUint8(t->getAe());
            stream.writeUint8(t->getPrefixLen());
            stream.writeUint16Be(t->getSeqno());
            stream.writeUint8(t->getHopcount());
            stream.writeUint8(t->getReserved());
            stream.writeUint32Be(t->getRouterIdHi());
            stream.writeUint32Be(t->getRouterIdLo());
            writePrefix(stream, t->getAe(), t->getPrefix(), t->getPrefixLen());
            break;
        }
        default:
            throw cRuntimeError("BabelSerializer: cannot serialize unknown TLV type %d", type);
    }
}

const Ptr<Chunk> BabelTlvSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readUint8();
    if (type == BABEL_PAD1) {
        auto t = makeShared<BabelPad1Tlv>();
        t->setTlvType(type);
        return t;
    }

    uint8_t bodyLen = stream.readUint8();

    switch (type) {
        case BABEL_PADN: {
            auto t = makeShared<BabelPadNTlv>();
            t->setN(bodyLen);
            for (uint8_t i = 0; i < bodyLen; i++)
                stream.readByte();
            t->setChunkLength(B(2) + B(bodyLen));
            return t;
        }
        case BABEL_ACKREQ: {
            auto t = makeShared<BabelAckReqTlv>();
            t->setReserved(stream.readUint16Be());
            t->setNonce(stream.readUint16Be());
            t->setInterval(stream.readUint16Be());
            return t;
        }
        case BABEL_ACK: {
            auto t = makeShared<BabelAckTlv>();
            t->setNonce(stream.readUint16Be());
            return t;
        }
        case BABEL_HELLO: {
            auto t = makeShared<BabelHelloTlv>();
            t->setReserved(stream.readUint16Be());
            t->setSeqno(stream.readUint16Be());
            t->setInterval(stream.readUint16Be());
            return t;
        }
        case BABEL_IHU: {
            auto t = makeShared<BabelIhuTlv>();
            uint8_t ae = stream.readUint8();
            t->setAe(ae);
            t->setReserved(stream.readUint8());
            t->setRxcost(stream.readUint16Be());
            t->setInterval(stream.readUint16Be());
            t->setAddress(readAddress(stream, ae));
            t->setChunkLength(B(8) + B(AE::maxLen(ae)));
            return t;
        }
        case BABEL_ROUTERID: {
            auto t = makeShared<BabelRouterIdTlv>();
            t->setReserved(stream.readUint16Be());
            t->setRouterIdHi(stream.readUint32Be());
            t->setRouterIdLo(stream.readUint32Be());
            return t;
        }
        case BABEL_NEXTHOP: {
            auto t = makeShared<BabelNextHopTlv>();
            uint8_t ae = stream.readUint8();
            t->setAe(ae);
            t->setReserved(stream.readUint8());
            t->setNextHop(readAddress(stream, ae));
            t->setChunkLength(B(4) + B(AE::maxLen(ae)));
            return t;
        }
        case BABEL_UPDATE: {
            auto t = makeShared<BabelUpdateTlv>();
            uint8_t ae = stream.readUint8();
            t->setAe(ae);
            t->setFlags(stream.readUint8());
            uint8_t plen = stream.readUint8();
            uint8_t omitted = stream.readUint8();
            if (omitted != 0)
                throw cRuntimeError("BabelSerializer: Update prefix compression (omitted=%d) is not supported on receive", omitted);
            t->setInterval(stream.readUint16Be());
            t->setSeqno(stream.readUint16Be());
            t->setMetric(stream.readUint16Be());
            t->setPrefix(readPrefix(stream, ae, plen));
            t->setPrefixLen(plen);
            t->setChunkLength(B(12) + B(bitsToBytesLen(plen)));
            return t;
        }
        case BABEL_ROUTEREQ: {
            auto t = makeShared<BabelRouteReqTlv>();
            uint8_t ae = stream.readUint8();
            t->setAe(ae);
            uint8_t plen = stream.readUint8();
            t->setPrefix(readPrefix(stream, ae, plen));
            t->setPrefixLen(plen);
            t->setChunkLength(B(4) + B(bitsToBytesLen(plen)));
            return t;
        }
        case BABEL_SEQNOREQ: {
            auto t = makeShared<BabelSeqnoReqTlv>();
            uint8_t ae = stream.readUint8();
            t->setAe(ae);
            uint8_t plen = stream.readUint8();
            t->setSeqno(stream.readUint16Be());
            t->setHopcount(stream.readUint8());
            t->setReserved(stream.readUint8());
            t->setRouterIdHi(stream.readUint32Be());
            t->setRouterIdLo(stream.readUint32Be());
            t->setPrefix(readPrefix(stream, ae, plen));
            t->setPrefixLen(plen);
            t->setChunkLength(B(16) + B(bitsToBytesLen(plen)));
            return t;
        }
        default:
            throw cRuntimeError("BabelSerializer: cannot deserialize unknown TLV type %d", type);
    }
}

} // namespace babel
} // namespace inet
