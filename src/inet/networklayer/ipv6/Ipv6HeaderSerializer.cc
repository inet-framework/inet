//
// Copyright (C) 2013 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6HeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/TlvOptions_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/headers/ip6.h"
#include "inet/networklayer/xmipv6/MobilityHeader_m.h"

#if defined(_MSC_VER)
#undef s_addr /* MSVC #definition interferes with us */
#endif // if defined(_MSC_VER)

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h> // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr

namespace inet {

Register_Serializer(Ipv6Header, Ipv6HeaderSerializer);

static void serializeIpv6TlvOptions(MemoryOutputStream& stream, const TlvOptions& tlvOptions, B hdrLen)
{
    b startPos = stream.getLength();
    for (size_t i = 0; i < tlvOptions.getTlvOptionArraySize(); i++) {
        const TlvOptionBase *opt = tlvOptions.getTlvOption(i);
        if (opt->getType() == IPv6TLVOPTION_NOP1) {
            stream.writeByte(0); // Pad1: single zero byte, no length field
        }
        else if (opt->getType() == IPv6TLVOPTION_NOPN) {
            stream.writeByte(opt->getType());
            stream.writeByte(opt->getLength());
            stream.writeByteRepeatedly(0, opt->getLength());
        }
        else if (opt->getType() == IPv6TLVOPTION_HOME_ADDRESS) {
            auto *hao = check_and_cast<const HomeAddressOption *>(opt);
            stream.writeByte(hao->getType());
            stream.writeByte(16); // length: 16 bytes for IPv6 address
            stream.writeIpv6Address(hao->getHomeAddress());
        }
        else if (auto *raw = dynamic_cast<const TlvOptionRaw *>(opt)) {
            stream.writeByte(raw->getType());
            stream.writeByte(raw->getBytesArraySize());
            for (size_t j = 0; j < raw->getBytesArraySize(); j++)
                stream.writeByte(raw->getBytes(j));
        }
        else {
            throw cRuntimeError("Cannot serialize IPv6 TLV option type %d", opt->getType());
        }
    }
    // Pad to fill the remaining header bytes (hdrLen is total minus the 2-byte next-hdr+len prefix)
    b written = stream.getLength() - startPos;
    if (written < b(hdrLen))
        stream.writeByteRepeatedly(0, B(b(hdrLen) - written).get());
}

static void deserializeIpv6TlvOptions(MemoryInputStream& stream, TlvOptions& tlvOptions, B optionsLen)
{
    b startPos = stream.getPosition();
    while (stream.getPosition() - startPos < b(optionsLen)) {
        uint8_t type = stream.readByte();
        if (type == IPv6TLVOPTION_NOP1) {
            auto *opt = new TlvOptionBase();
            opt->setType(IPv6TLVOPTION_NOP1);
            opt->setLength(0);
            tlvOptions.appendTlvOption(opt);
        }
        else {
            uint8_t length = stream.readByte();
            if (type == IPv6TLVOPTION_NOPN) {
                auto *opt = new TlvOptionBase();
                opt->setType(IPv6TLVOPTION_NOPN);
                opt->setLength(length);
                stream.readByteRepeatedly(0, length);
                tlvOptions.appendTlvOption(opt);
            }
            else if (type == IPv6TLVOPTION_HOME_ADDRESS) {
                auto *hao = new HomeAddressOption();
                hao->setLength(length);
                hao->setHomeAddress(stream.readIpv6Address());
                tlvOptions.appendTlvOption(hao);
            }
            else {
                auto *raw = new TlvOptionRaw();
                raw->setType(type);
                raw->setLength(length);
                raw->setBytesArraySize(length);
                for (int j = 0; j < length; j++)
                    raw->setBytes(j, stream.readByte());
                tlvOptions.appendTlvOption(raw);
            }
        }
    }
}

void Ipv6HeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ipv6Header = staticPtrCast<const Ipv6Header>(chunk);
    stream.writeUint4(ipv6Header->getVersion());
    stream.writeUint8(ipv6Header->getTrafficClass());
    stream.writeNBitsOfUint64Be(ipv6Header->getFlowLabel(), 20);
    stream.writeUint16Be(ipv6Header->getPayloadLength().get<B>());
    stream.writeByte(ipv6Header->getExtensionHeaderArraySize() != 0 ? ipv6Header->getExtensionHeader(0)->getExtensionType() : ipv6Header->getProtocolId());
    stream.writeByte(ipv6Header->getHopLimit());
    stream.writeIpv6Address(ipv6Header->getSrcAddress());
    stream.writeIpv6Address(ipv6Header->getDestAddress());
    // FIXME serialize extension headers
    for (size_t i = 0; i < ipv6Header->getExtensionHeaderArraySize(); i++) {
        b nextHdrCodePos = stream.getLength();
        const Ipv6ExtensionHeader *extHdr = ipv6Header->getExtensionHeader(i);
        stream.writeByte(i + 1 < ipv6Header->getExtensionHeaderArraySize() ? ipv6Header->getExtensionHeader(i + 1)->getExtensionType() : ipv6Header->getProtocolId());
        ASSERT((extHdr->getByteLength().get<B>() & 7) == 0);
        ASSERT(extHdr->getByteLength() <= B(255 * 8 + 8));
        stream.writeByte((extHdr->getByteLength().get<B>() - 8) / 8);
        switch (extHdr->getExtensionType()) {
            case IP_PROT_IPv6EXT_HOP: {
                const Ipv6HopByHopOptionsHeader *hdr = check_and_cast<const Ipv6HopByHopOptionsHeader *>(extHdr);
                serializeIpv6TlvOptions(stream, hdr->getTlvOptions(), hdr->getByteLength() - B(2));
                break;
            }
            case IP_PROT_IPv6EXT_DEST: {
                const Ipv6DestinationOptionsHeader *hdr = check_and_cast<const Ipv6DestinationOptionsHeader *>(extHdr);
                serializeIpv6TlvOptions(stream, hdr->getTlvOptions(), hdr->getByteLength() - B(2));
                break;
            }
            case IP_PROT_IPv6EXT_ROUTING: {
                const Ipv6RoutingHeader *hdr = check_and_cast<const Ipv6RoutingHeader *>(extHdr);
                stream.writeByte(hdr->getRoutingType());
                stream.writeByte(hdr->getSegmentsLeft());
                for (unsigned int j = 0; j < hdr->getAddressArraySize(); j++) {
                    stream.writeIpv6Address(hdr->getAddress(j));
                }
                stream.writeByteRepeatedly(0, 4);
                break;
            }
            case IP_PROT_IPv6EXT_FRAGMENT: {
                const Ipv6FragmentHeader *hdr = check_and_cast<const Ipv6FragmentHeader *>(extHdr);
                ASSERT(hdr->getByteLength() == IPv6_FRAGMENT_HEADER_LENGTH);
                ASSERT((hdr->getFragmentOffset() & 7) == 0);
                stream.writeNBitsOfUint64Be(hdr->getFragmentOffset() / 8, 13);
                stream.writeUint2(hdr->getReserved());
                stream.writeBit(hdr->getMoreFragments());
                stream.writeUint32Be(hdr->getIdentification());
                break;
            }
            case IP_PROT_IPv6EXT_AUTH: {
                const Ipv6AuthenticationHeader *hdr = check_and_cast<const Ipv6AuthenticationHeader *>(extHdr);
                stream.writeByteRepeatedly(0, hdr->getByteLength().get<B>() - 2); // TODO
                break;
            }
            case IP_PROT_IPv6EXT_ESP: {
                const Ipv6EncapsulatingSecurityPayloadHeader *hdr = check_and_cast<const Ipv6EncapsulatingSecurityPayloadHeader *>(extHdr);
                stream.writeByteRepeatedly(0, hdr->getByteLength().get<B>() - 2); // TODO
                break;
            }
            case IP_PROT_IPv6EXT_MOB: {
                stream.writeByteRepeatedly(0, extHdr->getByteLength().get<B>() - 2); // TODO
                break;
            }
            default: {
                throw cRuntimeError("Unknown Ipv6 extension header %d (%s)%s", extHdr->getExtensionType(), extHdr->getClassName(), extHdr->getFullName());
                break;
            }
        }
        ASSERT(nextHdrCodePos + b(extHdr->getByteLength()) == stream.getLength());
    }
}

const Ptr<Chunk> Ipv6HeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ipv6Header = makeShared<Ipv6Header>();
    ipv6Header->setVersion(stream.readUint4());
    if (ipv6Header->getVersion() != 6)
        ipv6Header->markIncorrect();
    ipv6Header->setTrafficClass(stream.readUint8());
    ipv6Header->setFlowLabel(stream.readNBitsToUint64Be(20));
    ipv6Header->setPayloadLength(B(stream.readUint16Be()));
    IpProtocolId nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
    ipv6Header->setProtocolId(nextHeaderId);
    ipv6Header->setHopLimit(stream.readByte());
    ipv6Header->setSrcAddress(stream.readIpv6Address());
    ipv6Header->setDestAddress(stream.readIpv6Address());
    bool mbool = true;
    while (mbool) {
        switch (nextHeaderId) {
            case IP_PROT_IPv6EXT_HOP: {
                Ipv6HopByHopOptionsHeader *extHdr = new Ipv6HopByHopOptionsHeader();
                extHdr->setExtensionType(nextHeaderId);
                nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
                uint8_t hdrExtLen = stream.readByte();
                uint16_t hdrLen = hdrExtLen * 8 + 8;
                extHdr->setByteLength(B(hdrLen));
                deserializeIpv6TlvOptions(stream, extHdr->getTlvOptionsForUpdate(), B(hdrLen - 2));
                ipv6Header->appendExtensionHeader(extHdr);
                break;
            }
            case IP_PROT_IPv6EXT_DEST: {
                Ipv6DestinationOptionsHeader *extHdr = new Ipv6DestinationOptionsHeader();
                extHdr->setExtensionType(nextHeaderId);
                nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
                uint8_t hdrExtLen = stream.readByte();
                uint16_t hdrLen = hdrExtLen * 8 + 8;
                extHdr->setByteLength(B(hdrLen));
                deserializeIpv6TlvOptions(stream, extHdr->getTlvOptionsForUpdate(), B(hdrLen - 2));
                ipv6Header->appendExtensionHeader(extHdr);
                break;
            }
            case IP_PROT_IPv6EXT_ROUTING: {
                Ipv6RoutingHeader *extHdr = new Ipv6RoutingHeader();
                extHdr->setExtensionType(nextHeaderId);
                nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
                uint8_t hdrExtLen = stream.readByte();
                uint16_t hdrLen = hdrExtLen * 8 + 8;
                extHdr->setByteLength(B(hdrLen));
                extHdr->setRoutingType(stream.readByte());
                extHdr->setSegmentsLeft(stream.readByte());
                stream.readByteRepeatedly(0, hdrLen - 2); // TODO
                ipv6Header->appendExtensionHeader(extHdr);
                break;
            }
            case IP_PROT_IPv6EXT_FRAGMENT: {
                Ipv6FragmentHeader *extHdr = new Ipv6FragmentHeader();
                extHdr->setExtensionType(nextHeaderId);
                nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
                // hdrExtLen ignored, see RFC2460: it's only a reserved field for fragment header
                stream.readByte();
                extHdr->setByteLength(IPv6_FRAGMENT_HEADER_LENGTH);
                extHdr->setFragmentOffset(stream.readNBitsToUint64Be(13) * 8);
                extHdr->setReserved(stream.readUint2());
                extHdr->setMoreFragments(stream.readBit());
                extHdr->setIdentification(stream.readUint32Be());
                ipv6Header->appendExtensionHeader(extHdr);
                break;
            }
            case IP_PROT_IPv6EXT_AUTH: {
                Ipv6AuthenticationHeader *extHdr = new Ipv6AuthenticationHeader();
                extHdr->setExtensionType(nextHeaderId);
                nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
                uint8_t hdrExtLen = stream.readByte();
                uint16_t hdrLen = hdrExtLen * 8 + 8;
                extHdr->setByteLength(B(hdrLen));
                stream.readByteRepeatedly(0, hdrLen - 2); // TODO
                ipv6Header->appendExtensionHeader(extHdr);
                break;
            }
            case IP_PROT_IPv6EXT_ESP: {
                Ipv6EncapsulatingSecurityPayloadHeader *extHdr = new Ipv6EncapsulatingSecurityPayloadHeader();
                extHdr->setExtensionType(nextHeaderId);
                nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
                uint8_t hdrExtLen = stream.readByte();
                uint16_t hdrLen = hdrExtLen * 8 + 8;
                extHdr->setByteLength(B(hdrLen));
                stream.readByteRepeatedly(0, hdrLen - 2); // TODO
                ipv6Header->appendExtensionHeader(extHdr);
                break;
            }
            case IP_PROT_IPv6EXT_MOB: {
                Ipv6ExtensionHeader *extHdr = new Ipv6ExtensionHeader();
                extHdr->setExtensionType(nextHeaderId);
                nextHeaderId = static_cast<IpProtocolId>(stream.readByte());
                uint8_t hdrExtLen = stream.readByte();
                uint16_t hdrLen = hdrExtLen * 8 + 8;
                extHdr->setByteLength(B(hdrLen));
                stream.readByteRepeatedly(0, hdrLen - 2); // TODO
                ipv6Header->appendExtensionHeader(extHdr);
                break;
            }
            default: {
                mbool = false;
                break;
            }
        }
    }
    return ipv6Header;
}

} // namespace inet

