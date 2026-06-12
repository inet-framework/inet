//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6/Ipv6ExtensionHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/TlvOptions_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"

namespace inet {

Register_Serializer(Ipv6HopByHopOptionsHeader, Ipv6HopByHopOptionsHeaderSerializer);
Register_Serializer(Ipv6DestinationOptionsHeader, Ipv6DestinationOptionsHeaderSerializer);
Register_Serializer(Ipv6RoutingHeader, Ipv6RoutingHeaderSerializer);
Register_Serializer(Ipv6FragmentHeader, Ipv6FragmentHeaderSerializer);
Register_Serializer(Ipv6AuthenticationHeader, Ipv6AuthenticationHeaderSerializer);
Register_Serializer(Ipv6EncapsulatingSecurityPayloadHeader, Ipv6EncapsulatingSecurityPayloadHeaderSerializer);

// ---- TLV option helpers (shared by HopByHop and DestOpts) ----

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

// ---- HopByHop Options Header ----

void Ipv6HopByHopOptionsHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& hdr = staticPtrCast<const Ipv6HopByHopOptionsHeader>(chunk);
    stream.writeByte(hdr->getNextHeaderProtocol());
    B totalLen = hdr->getChunkLength();
    ASSERT(totalLen.get() % 8 == 0 && totalLen >= B(8));
    stream.writeByte((totalLen.get() - 8) / 8);
    serializeIpv6TlvOptions(stream, hdr->getTlvOptions(), totalLen - B(2));
}

const Ptr<Chunk> Ipv6HopByHopOptionsHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto hdr = makeShared<Ipv6HopByHopOptionsHeader>();
    hdr->setNextHeaderProtocol(static_cast<IpProtocolId>(stream.readByte()));
    uint8_t hdrExtLen = stream.readByte();
    B totalLen = B(hdrExtLen * 8 + 8);
    hdr->setChunkLength(totalLen);
    deserializeIpv6TlvOptions(stream, hdr->getTlvOptionsForUpdate(), totalLen - B(2));
    return hdr;
}

// ---- Destination Options Header ----

void Ipv6DestinationOptionsHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& hdr = staticPtrCast<const Ipv6DestinationOptionsHeader>(chunk);
    stream.writeByte(hdr->getNextHeaderProtocol());
    B totalLen = hdr->getChunkLength();
    ASSERT(totalLen.get() % 8 == 0 && totalLen >= B(8));
    stream.writeByte((totalLen.get() - 8) / 8);
    serializeIpv6TlvOptions(stream, hdr->getTlvOptions(), totalLen - B(2));
}

const Ptr<Chunk> Ipv6DestinationOptionsHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto hdr = makeShared<Ipv6DestinationOptionsHeader>();
    hdr->setNextHeaderProtocol(static_cast<IpProtocolId>(stream.readByte()));
    uint8_t hdrExtLen = stream.readByte();
    B totalLen = B(hdrExtLen * 8 + 8);
    hdr->setChunkLength(totalLen);
    deserializeIpv6TlvOptions(stream, hdr->getTlvOptionsForUpdate(), totalLen - B(2));
    return hdr;
}

// ---- Routing Header ----

void Ipv6RoutingHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& hdr = staticPtrCast<const Ipv6RoutingHeader>(chunk);
    stream.writeByte(hdr->getNextHeaderProtocol());
    B totalLen = hdr->getChunkLength();
    ASSERT(totalLen.get() % 8 == 0 && totalLen >= B(8));
    stream.writeByte((totalLen.get() - 8) / 8);
    stream.writeByte(hdr->getRoutingType());
    stream.writeByte(hdr->getSegmentsLeft());
    stream.writeByteRepeatedly(0, 4); // reserved
    for (size_t j = 0; j < hdr->getAddressArraySize(); j++)
        stream.writeIpv6Address(hdr->getAddress(j));
}

const Ptr<Chunk> Ipv6RoutingHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto hdr = makeShared<Ipv6RoutingHeader>();
    hdr->setNextHeaderProtocol(static_cast<IpProtocolId>(stream.readByte()));
    uint8_t hdrExtLen = stream.readByte();
    B totalLen = B(hdrExtLen * 8 + 8);
    hdr->setChunkLength(totalLen);
    hdr->setRoutingType(stream.readByte());
    hdr->setSegmentsLeft(stream.readByte());
    stream.readByteRepeatedly(0, 4); // reserved
    // Remaining bytes are addresses (16 bytes each)
    int numAddresses = (totalLen.get() - 8) / 16;
    hdr->setAddressArraySize(numAddresses);
    for (int j = 0; j < numAddresses; j++)
        hdr->setAddress(j, stream.readIpv6Address());
    return hdr;
}

// ---- Fragment Header ----

void Ipv6FragmentHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& hdr = staticPtrCast<const Ipv6FragmentHeader>(chunk);
    stream.writeByte(hdr->getNextHeaderProtocol());
    stream.writeByte(0); // reserved
    ASSERT((hdr->getFragmentOffset() & 7) == 0);
    stream.writeNBitsOfUint64Be(hdr->getFragmentOffset() / 8, 13);
    stream.writeUint2(hdr->getReserved());
    stream.writeBit(hdr->getMoreFragments());
    stream.writeUint32Be(hdr->getIdentification());
}

const Ptr<Chunk> Ipv6FragmentHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto hdr = makeShared<Ipv6FragmentHeader>();
    hdr->setNextHeaderProtocol(static_cast<IpProtocolId>(stream.readByte()));
    stream.readByte(); // reserved
    hdr->setFragmentOffset(stream.readNBitsToUint64Be(13) * 8);
    hdr->setReserved(stream.readUint2());
    hdr->setMoreFragments(stream.readBit());
    hdr->setIdentification(stream.readUint32Be());
    return hdr;
}

// ---- Authentication Header ----

void Ipv6AuthenticationHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& hdr = staticPtrCast<const Ipv6AuthenticationHeader>(chunk);
    stream.writeByte(hdr->getNextHeaderProtocol());
    B totalLen = hdr->getChunkLength();
    ASSERT(totalLen.get() % 8 == 0 && totalLen >= B(8));
    stream.writeByte((totalLen.get() - 8) / 8);
    stream.writeByteRepeatedly(0, totalLen.get() - 2); // TODO
}

const Ptr<Chunk> Ipv6AuthenticationHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto hdr = makeShared<Ipv6AuthenticationHeader>();
    hdr->setNextHeaderProtocol(static_cast<IpProtocolId>(stream.readByte()));
    uint8_t hdrExtLen = stream.readByte();
    B totalLen = B(hdrExtLen * 8 + 8);
    hdr->setChunkLength(totalLen);
    stream.readByteRepeatedly(0, totalLen.get() - 2); // TODO
    return hdr;
}

// ---- ESP Header ----

void Ipv6EncapsulatingSecurityPayloadHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& hdr = staticPtrCast<const Ipv6EncapsulatingSecurityPayloadHeader>(chunk);
    stream.writeByte(hdr->getNextHeaderProtocol());
    B totalLen = hdr->getChunkLength();
    ASSERT(totalLen.get() % 8 == 0 && totalLen >= B(8));
    stream.writeByte((totalLen.get() - 8) / 8);
    stream.writeByteRepeatedly(0, totalLen.get() - 2); // TODO
}

const Ptr<Chunk> Ipv6EncapsulatingSecurityPayloadHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto hdr = makeShared<Ipv6EncapsulatingSecurityPayloadHeader>();
    hdr->setNextHeaderProtocol(static_cast<IpProtocolId>(stream.readByte()));
    uint8_t hdrExtLen = stream.readByte();
    B totalLen = B(hdrExtLen * 8 + 8);
    hdr->setChunkLength(totalLen);
    stream.readByteRepeatedly(0, totalLen.get() - 2); // TODO
    return hdr;
}

} // namespace inet
