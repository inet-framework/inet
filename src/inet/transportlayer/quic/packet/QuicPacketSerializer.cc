//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/transportlayer/quic/packet/QuicPacketSerializer.h"

#include <iomanip>

#include "inet/transportlayer/quic/packet/QuicPacket.h"
#include "inet/transportlayer/quic/packet/EncryptedQuicPacketChunk.h"
#include "inet/transportlayer/quic/packet/EncryptionKeyTag_m.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/Endian.h"


extern "C" {
#include "picotls.h"
#include "picotls/openssl_opp.h"
}

namespace inet {
namespace quic {

Register_Serializer(PacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(LongPacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(ShortPacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(InitialPacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(HandshakePacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(ZeroRttPacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(RetryPacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(OneRttPacketHeader, QuicPacketHeaderSerializer);
Register_Serializer(VersionNegotiationPacketHeader, QuicPacketHeaderSerializer);

Register_Serializer(EncryptedQuicPacketChunk, EncryptedQuicPacketSerializer);

//
// QuicPacketHeaderSerializer
//

void QuicPacketHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const PacketHeader>(chunk);
    uint8_t headerForm = header->getHeaderForm();

    if (headerForm == PACKET_HEADER_FORM_LONG) {
        const auto& longHeader = staticPtrCast<const LongPacketHeader>(header);
        // Determine the specific type of long packet header
        if (longHeader->getVersion() == 0) {
            // Version Negotiation Packet
            const auto& versionNegotiationHeader = staticPtrCast<const VersionNegotiationPacketHeader>(longHeader);
            serializeVersionNegotiationPacketHeader(stream, versionNegotiationHeader);
        }
        else {
            switch (longHeader->getLongPacketType()) {
                case LONG_PACKET_HEADER_TYPE_INITIAL: {
                Ptr<const InitialPacketHeader> initialPacketHeader = staticPtrCast<const InitialPacketHeader>(longHeader);
                    serializeInitialPacketHeader(stream, initialPacketHeader);
                }
                    break;
                case LONG_PACKET_HEADER_TYPE_0RTT:
                    serializeZeroRttPacketHeader(stream, staticPtrCast<const ZeroRttPacketHeader>(longHeader));
                    break;
                case LONG_PACKET_HEADER_TYPE_HANDSHAKE:
                    serializeHandshakePacketHeader(stream, staticPtrCast<const HandshakePacketHeader>(longHeader));
                    break;
                case LONG_PACKET_HEADER_TYPE_RETRY:
                    serializeRetryPacketHeader(stream, staticPtrCast<const RetryPacketHeader>(longHeader));
                    break;
                default:
                    throw cRuntimeError("Unknown long packet type: %d", longHeader->getLongPacketType());
            }
        }
    }
    else {
        // Short header
        const auto& shortHeader = staticPtrCast<const ShortPacketHeader>(header);

        // Check if it's a OneRttPacketHeader
        if (dynamic_cast<const OneRttPacketHeader*>(header.get())) {
            serializeOneRttPacketHeader(stream, staticPtrCast<const OneRttPacketHeader>(shortHeader));
        }
        else {
            serializeShortPacketHeader(stream, shortHeader);
        }
    }
}

void QuicPacketHeaderSerializer::serializeLongPacketHeader(MemoryOutputStream& stream, const Ptr<const LongPacketHeader>& header, uint8_t typeSpecificBits) const
{
    std::cout << "serializeLongPacketHeader begin, stream length: " << stream.getLength() << std::endl;
    ASSERT((typeSpecificBits & 0xfc) == 0); // Ensure that the type-specific bits are in the correct range
    // First byte: Header form (1 bit) | Reserved (1 bit) | Packet type (2 bits) | Type-specific bits (4 bits)
    uint8_t firstByte = (header->getHeaderForm() << 7) | (1 << 6) | (header->getLongPacketType() << 4) | typeSpecificBits;
    stream.writeByte(firstByte);

    // Version
    stream.writeUint32Be(header->getVersion());

    std::cout << "dst conn id len: " << (int)header->getDstConnectionIdLength() << std::endl;
    // Destination Connection ID Length
    stream.writeByte(header->getDstConnectionIdLength());

    // Destination Connection ID
    if (header->getDstConnectionIdLength() > 0) {
        stream.writeUint64Be(header->getDstConnectionId());
    }

    std::cout << "src conn id len: " << (int)header->getSrcConnectionIdLength() << std::endl;
    // Source Connection ID Length
    stream.writeByte(header->getSrcConnectionIdLength());

    // Source Connection ID
    if (header->getSrcConnectionIdLength() > 0) {
        stream.writeUint64Be(header->getSrcConnectionId());
    }

    std::cout << "serializeLongPacketHeader end, stream length: " << stream.getLength() << std::endl;
}

void QuicPacketHeaderSerializer::serializeShortPacketHeader(MemoryOutputStream& stream, const Ptr<const ShortPacketHeader>& header) const
{
    // First byte: Header form (1 bit) | Fixed (1 bit) | Spin bit (1 bit) | Reserved (3 bits) | Key phase (1 bit) | Packet number length (2 bits)
    uint8_t firstByte = (header->getHeaderForm() << 7) | 0x40; // Fixed bit is always 1
    stream.writeByte(firstByte);

    // Destination Connection ID
    stream.writeUint64Be(header->getDstConnectionId());

    // Packet Number
    stream.writeUint32Be(header->getPacketNumber());
}

void QuicPacketHeaderSerializer::serializeInitialPacketHeader(MemoryOutputStream& stream, const Ptr<const InitialPacketHeader>& header) const
{
    std::cout << "serializeInitialPacketHeader begin, stream length: " << stream.getLength() << std::endl;
    // Serialize the common long header fields
    std::cout << "dst conn id len: " << (int)header->getDstConnectionIdLength() << std::endl;
    std::cout << "dst conn id: " << header->getDstConnectionId() << std::endl;
    std::cout << "src conn id len: " << (int)header->getSrcConnectionIdLength() << std::endl;
    std::cout << "src conn id: " << header->getSrcConnectionId() << std::endl;
    std::cout << "packet num len: " << (int)header->getPacketNumberLength() << std::endl;
    serializeLongPacketHeader(stream, header, std::log2(header->getPacketNumberLength()));

    // Token Length
    serializeVariableLengthInteger(stream, header->getTokenLength());

    // Token (if present)
    if (header->getTokenLength() > 0) {
        stream.writeUint64Be(header->getToken());
    }

    std::cout << "len: " << header->getLength() << std::endl;
    // Length
    serializeVariableLengthInteger(stream, header->getLength());

    // Packet Number
    stream.writeByte(0); // initial packet number is 1
    std::cout << "serializeInitialPacketHeader end, stream length: " << stream.getLength() << std::endl;
}

void QuicPacketHeaderSerializer::serializeHandshakePacketHeader(MemoryOutputStream& stream, const Ptr<const HandshakePacketHeader>& header) const
{
    // Serialize the common long header fields
    serializeLongPacketHeader(stream, header, std::log2(header->getPacketNumberLength()));

    // Length
    serializeVariableLengthInteger(stream, header->getLength());

    // Packet Number
    serializeVariableLengthInteger(stream, header->getPacketNumber());
}

void QuicPacketHeaderSerializer::serializeZeroRttPacketHeader(MemoryOutputStream& stream, const Ptr<const ZeroRttPacketHeader>& header) const
{
    // Serialize the common long header fields
    serializeLongPacketHeader(stream, header, std::log2(header->getPacketNumberLength()));

    // Length
    serializeVariableLengthInteger(stream, header->getLength());

    // Packet Number
    stream.writeUint32Be(header->getPacketNumber());
}

void QuicPacketHeaderSerializer::serializeRetryPacketHeader(MemoryOutputStream& stream, const Ptr<const RetryPacketHeader>& header) const
{
    // Serialize the common long header fields
    serializeLongPacketHeader(stream, header, 0);

    // Retry Token
    stream.writeUint64Be(header->getRetryToken());

    // Retry Integrity Tag
    stream.writeUint64Be(header->getRetryIntegrityTag());
}

void QuicPacketHeaderSerializer::serializeOneRttPacketHeader(MemoryOutputStream& stream, const Ptr<const OneRttPacketHeader>& header) const
{
    // First byte: Header form (1 bit) | Fixed (1 bit) | Spin bit (1 bit) | Reserved (3 bits) | Key phase (1 bit) | Packet number length (2 bits)
    uint8_t firstByte = (header->getHeaderForm() << 7) | 0x40; // Fixed bit is always 1
    if (header->getIBit()) {
        firstByte |= 0x20; // Set the I bit
    }
    stream.writeByte(firstByte);

    // Destination Connection ID
    stream.writeUint64Be(header->getDstConnectionId());

    // Packet Number
    stream.writeUint32Be(header->getPacketNumber());
}

void QuicPacketHeaderSerializer::serializeVersionNegotiationPacketHeader(MemoryOutputStream& stream, const Ptr<const VersionNegotiationPacketHeader>& header) const
{
    // First byte: Header form (1 bit) | Unused (7 bits)
    uint8_t firstByte = (header->getHeaderForm() << 7) | 0x7F; // All 1s for unused bits
    stream.writeByte(firstByte);

    // Version (0 for Version Negotiation)
    stream.writeUint32Be(0);

    // Destination Connection ID Length
    stream.writeByte(header->getDstConnectionIdLength());

    // Destination Connection ID
    if (header->getDstConnectionIdLength() > 0) {
        stream.writeUint64Be(header->getDstConnectionId());
    }

    // Source Connection ID Length
    stream.writeByte(header->getSrcConnectionIdLength());

    // Source Connection ID
    if (header->getSrcConnectionIdLength() > 0) {
        stream.writeUint64Be(header->getSrcConnectionId());
    }

    // Supported Versions
    for (size_t i = 0; i < header->getSupportedVersionArraySize(); i++) {
        stream.writeUint32Be(header->getSupportedVersion(i));
    }
}

const Ptr<Chunk> QuicPacketHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t firstByte = stream.readByte();
    uint8_t headerForm = (firstByte >> 7) & 0x01;

    if (headerForm == PACKET_HEADER_FORM_LONG) {
        return deserializeLongPacketHeader(stream, firstByte);
    }
    else {
        return deserializeShortPacketHeader(stream, firstByte);
    }
}

const Ptr<Chunk> QuicPacketHeaderSerializer::deserializeLongPacketHeader(MemoryInputStream& stream, uint8_t firstByte) const
{
    // Read version
    uint32_t version = stream.readUint32Be();

    if (version == 0) {
        // Version Negotiation Packet
        auto header = makeShared<VersionNegotiationPacketHeader>();
        header->setHeaderForm(PACKET_HEADER_FORM_LONG);

        // Destination Connection ID Length
        uint8_t dstConnectionIdLength = stream.readByte();
        header->setDstConnectionIdLength(dstConnectionIdLength);

        // Destination Connection ID
        if (dstConnectionIdLength > 0) {
            header->setDstConnectionId(stream.readUint64Be());
        }

        // Source Connection ID Length
        uint8_t srcConnectionIdLength = stream.readByte();
        header->setSrcConnectionIdLength(srcConnectionIdLength);

        // Source Connection ID
        if (srcConnectionIdLength > 0) {
            header->setSrcConnectionId(stream.readUint64Be());
        }

        // Supported Versions
        std::vector<uint32_t> supportedVersions;
        while (stream.getRemainingLength() >= B(4)) {
            supportedVersions.push_back(stream.readUint32Be());
        }

        header->setSupportedVersionArraySize(supportedVersions.size());
        for (size_t i = 0; i < supportedVersions.size(); i++) {
            header->setSupportedVersion(i, supportedVersions[i]);
        }

        return header;
    }
    else {
        // Other Long Header Packet
        uint8_t packetType = (firstByte >> 4) & 0x03;

        // Destination Connection ID Length
        uint8_t dstConnectionIdLength = stream.readByte();

        // Destination Connection ID
        uint64_t dstConnectionId = 0;
        if (dstConnectionIdLength > 0) {
            dstConnectionId = stream.readUint64Be();
        }

        // Source Connection ID Length
        uint8_t srcConnectionIdLength = stream.readByte();

        // Source Connection ID
        uint64_t srcConnectionId = 0;
        if (srcConnectionIdLength > 0) {
            srcConnectionId = stream.readUint64Be();
        }

        switch (packetType) {
            case LONG_PACKET_HEADER_TYPE_INITIAL: {
                auto header = makeShared<InitialPacketHeader>();
                header->setHeaderForm(PACKET_HEADER_FORM_LONG);
                header->setLongPacketType(LONG_PACKET_HEADER_TYPE_INITIAL);
                header->setVersion(version);
                header->setDstConnectionIdLength(dstConnectionIdLength);
                header->setDstConnectionId(dstConnectionId);
                header->setSrcConnectionIdLength(srcConnectionIdLength);
                header->setSrcConnectionId(srcConnectionId);

                // Token Length
                uint64_t tokenLength = deserializeVariableLengthInteger(stream);
                header->setTokenLength(tokenLength);

                // Token
                if (tokenLength > 0) {
                    header->setToken(stream.readUint64Be());
                }

                // Length
                uint64_t length = deserializeVariableLengthInteger(stream);
                header->setLength(length);

                // Packet Number
                header->setPacketNumber(stream.readUint32Be());

                header->calcChunkLength();
                return header;
            }
            case LONG_PACKET_HEADER_TYPE_0RTT: {
                auto header = makeShared<ZeroRttPacketHeader>();
                header->setHeaderForm(PACKET_HEADER_FORM_LONG);
                header->setLongPacketType(LONG_PACKET_HEADER_TYPE_0RTT);
                header->setVersion(version);
                header->setDstConnectionIdLength(dstConnectionIdLength);
                header->setDstConnectionId(dstConnectionId);
                header->setSrcConnectionIdLength(srcConnectionIdLength);
                header->setSrcConnectionId(srcConnectionId);

                // Length
                uint64_t length = deserializeVariableLengthInteger(stream);
                header->setLength(length);

                // Packet Number
                header->setPacketNumber(stream.readUint32Be());

                return header;
            }
            case LONG_PACKET_HEADER_TYPE_HANDSHAKE: {
                auto header = makeShared<HandshakePacketHeader>();
                header->setHeaderForm(PACKET_HEADER_FORM_LONG);
                header->setLongPacketType(LONG_PACKET_HEADER_TYPE_HANDSHAKE);
                header->setVersion(version);
                header->setDstConnectionIdLength(dstConnectionIdLength);
                header->setDstConnectionId(dstConnectionId);
                header->setSrcConnectionIdLength(srcConnectionIdLength);
                header->setSrcConnectionId(srcConnectionId);

                // Length
                uint64_t length = deserializeVariableLengthInteger(stream);
                header->setLength(length);

                // Packet Number
                header->setPacketNumber(stream.readUint32Be());

                header->calcChunkLength();
                return header;
            }
            case LONG_PACKET_HEADER_TYPE_RETRY: {
                auto header = makeShared<RetryPacketHeader>();
                header->setHeaderForm(PACKET_HEADER_FORM_LONG);
                header->setLongPacketType(LONG_PACKET_HEADER_TYPE_RETRY);
                header->setVersion(version);
                header->setDstConnectionIdLength(dstConnectionIdLength);
                header->setDstConnectionId(dstConnectionId);
                header->setSrcConnectionIdLength(srcConnectionIdLength);
                header->setSrcConnectionId(srcConnectionId);

                // Retry Token
                header->setRetryToken(stream.readUint64Be());

                // Retry Integrity Tag
                header->setRetryIntegrityTag(stream.readUint64Be());

                return header;
            }
            default:
                throw cRuntimeError("Unknown long packet type: %d", packetType);
        }
    }
}

const Ptr<Chunk> QuicPacketHeaderSerializer::deserializeShortPacketHeader(MemoryInputStream& stream, uint8_t firstByte) const
{
    // Check if the I bit is set (for OneRttPacketHeader)
    bool iBit = (firstByte & 0x20) != 0;

    if (iBit) {
        auto header = makeShared<OneRttPacketHeader>();
        header->setHeaderForm(PACKET_HEADER_FORM_SHORT);
        header->setIBit(true);

        // Destination Connection ID
        header->setDstConnectionId(stream.readUint64Be());

        // Packet Number
        header->setPacketNumber(stream.readUint32Be());

        return header;
    }
    else {
        auto header = makeShared<ShortPacketHeader>();
        header->setHeaderForm(PACKET_HEADER_FORM_SHORT);

        // Destination Connection ID
        header->setDstConnectionId(stream.readUint64Be());

        // Packet Number
        header->setPacketNumber(stream.readUint32Be());

        return header;
    }
}

std::vector<uint8_t> protectPacket(std::vector<uint8_t> datagram, uint32_t packetNumber, size_t packetNumberOffset, size_t packetNumberLength, const EncryptionKey& key) {
    ptls_cipher_suite_t *cs = &ptls_openssl_opp_aes128gcmsha256;
    size_t originalSize = datagram.size();

    std::cout << "protectPacket: original size: " << originalSize << std::endl;
    std::cout << "protectPacket: packet number: " << packetNumber << std::endl;
    std::cout << "protectPacket: packet number offset: " << packetNumberOffset << std::endl;
    std::cout << "protectPacket: packet number length: " << packetNumberLength << std::endl;


    datagram.resize(originalSize + 16); // Ensure enough space for the auth tag
    // generate new AEAD context
    ptls_aead_context_t *packet_protect = ptls_aead_new_direct(cs->aead, true, key.key.data(), key.iv.data());
    ptls_cipher_context_t *header_protect = ptls_cipher_new(cs->aead->ctr_cipher, true, key.hpkey.data());

    /*

    ptls_aead_supplementary_encryption_t supp = {.ctx = header_protect_ctx,
                                                 .input = datagram.base + payload_from - QUICLY_SEND_PN_SIZE + QUICLY_MAX_PN_SIZE};

    ptls_aead_encrypt_s(packet_protect_ctx, datagram.base + payload_from, datagram.base + payload_from,
                        datagram.len - payload_from - packet_protect_ctx->algo->tag_size, packet_number,
                        datagram.base + first_byte_at, payload_from - first_byte_at, &supp);

     */
    size_t payload_from = packetNumberOffset + packetNumberLength;

    ptls_aead_supplementary_encryption_t supp = {.ctx = header_protect,
                                                 .input = datagram.data() + payload_from + 4 - packetNumberLength};

    ptls_aead_encrypt_s(packet_protect, datagram.data() + payload_from, datagram.data() + payload_from,
                                   originalSize - payload_from,
                                   packetNumber,
                                   datagram.data(), payload_from, &supp);

    // Apply header protection
    uint8_t headerForm = (datagram[0] >> 7) & 0x01;
    // mask is applied to the low 4 bits in long headers, and low 5 bits in long headers, of the first byte
    datagram[0] ^= supp.output[0] & ((headerForm == PACKET_HEADER_FORM_LONG) ? 0xf : 0x1f);

    for (size_t i = 0; i != packetNumberLength; ++i)
        datagram[packetNumberOffset + i] ^= supp.output[i + 1];

    return datagram;
}

void EncryptedQuicPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto payload = staticPtrCast<const EncryptedQuicPacketChunk>(chunk)->getChunk();
    b payloadLength = payload->getChunkLength();

    Ptr<const EncryptionKeyTag> keyTag = chunk->getTag<EncryptionKeyTag>();

    auto payloadSequence = dynamicPtrCast<SequenceChunk>(payload);
    ASSERT(payloadSequence != nullptr);
    auto payloadChunks = payloadSequence->getChunks();

    auto quicPacketHeader = dynamicPtrCast<const PacketHeader>(payloadChunks[0]);
    ASSERT(quicPacketHeader != nullptr);
    uint8_t packetNumberLength = 0;
    uint32_t packetNumber = 0;

    switch (quicPacketHeader->getHeaderForm()) {
        case PACKET_HEADER_FORM_LONG: {
            auto longPacketHeader = dynamicPtrCast<const LongPacketHeader>(quicPacketHeader);
            switch (longPacketHeader->getLongPacketType()) {
                case LONG_PACKET_HEADER_TYPE_INITIAL: {
                    auto initialPacketHeader = dynamicPtrCast<const InitialPacketHeader>(longPacketHeader);
                    ASSERT(initialPacketHeader != nullptr);
                    packetNumberLength = initialPacketHeader->getPacketNumberLength();
                    packetNumber = initialPacketHeader->getPacketNumber(); // Not used in this context
                }
                break;
                case LONG_PACKET_HEADER_TYPE_0RTT: {
                    auto zeroRttPacketHeader = dynamicPtrCast<const ZeroRttPacketHeader>(longPacketHeader);
                    ASSERT(zeroRttPacketHeader != nullptr);
                    packetNumberLength = zeroRttPacketHeader->getPacketNumberLength();
                    packetNumber = zeroRttPacketHeader->getPacketNumber();
                }
                break;
                case LONG_PACKET_HEADER_TYPE_HANDSHAKE: {
                    auto handshakePacketHeader = dynamicPtrCast<const HandshakePacketHeader>(longPacketHeader);
                    ASSERT(handshakePacketHeader != nullptr);
                    packetNumberLength = handshakePacketHeader->getPacketNumberLength();
                    packetNumber = handshakePacketHeader->getPacketNumber();
                }
            }
            break;
        }
        case PACKET_HEADER_FORM_SHORT: {
            auto shortPacketHeader = dynamicPtrCast<const ShortPacketHeader>(quicPacketHeader);
            ASSERT(shortPacketHeader != nullptr);
            //TODO packetNumberLength = shortPacketHeader->getPacketNumberLength();
            packetNumber = shortPacketHeader->getPacketNumber();
            break;
        }
        default:
            throw cRuntimeError("Unknown packet header form: %d", quicPacketHeader->getHeaderForm());
    }

    MemoryOutputStream payloadStream;

    const Chunk *payloadPointer = payload.get();
    auto serializer = ChunkSerializerRegistry::getInstance().getSerializer(typeid(*payloadPointer));
    serializer->serialize(payloadStream, payload, B(0), payloadLength);

    std::vector<uint8_t> unencryptedData = payloadStream.getData();
    payloadStream.clear();

    EncryptionKey key = EncryptionKey::fromTag(keyTag);
    std::vector<uint8_t> finalContents = protectPacket(unencryptedData, packetNumber, 26, packetNumberLength, key);

    // TODO: handle sub-byte offset and length
    stream.writeBytes(finalContents, offset, length < b(0) ? B(-1) : B(length));


    // ----------------

    uint8_t test_dcid[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    ptls_iovec_t test_dcid_iovec = ptls_iovec_init(test_dcid, 8);



    // self test from https://quic.xargs.org/#client-initial-packet

    std::vector<uint8_t> testClientUnprotectedData = {
        0xc0, // unprotected header byte
        0x00, 0x00, 0x00, 0x01, // version
        0x08, // dcid len
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // dcid
        0x05, // scid len
        0x63, 0x5f, 0x63, 0x69, 0x64, // scid
        0x00, // token
        0x41, 0x03, // packet length
        0x00, // unprotected packet number
        // crypto frame
        0x06, 0x00, 0x40, 0xee, 0x01, 0x00, 0x00, 0xea, 0x03, 0x03, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x00, 0x00,
        0x06, 0x13, 0x01, 0x13, 0x02, 0x13, 0x03, 0x01, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x18, 0x00, 0x16, 0x00, 0x00, 0x13, 0x65, 0x78,
        0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x75, 0x6c, 0x66, 0x68, 0x65, 0x69, 0x6d, 0x2e, 0x6e, 0x65, 0x74, 0x00, 0x0a, 0x00, 0x08, 0x00,
        0x06, 0x00, 0x1d, 0x00, 0x17, 0x00, 0x18, 0x00, 0x10, 0x00, 0x0b, 0x00, 0x09, 0x08, 0x70, 0x69, 0x6e, 0x67, 0x2f, 0x31, 0x2e, 0x30,
        0x00, 0x0d, 0x00, 0x14, 0x00, 0x12, 0x04, 0x03, 0x08, 0x04, 0x04, 0x01, 0x05, 0x03, 0x08, 0x05, 0x05, 0x01, 0x08, 0x06, 0x06, 0x01,
        0x02, 0x01, 0x00, 0x33, 0x00, 0x26, 0x00, 0x24, 0x00, 0x1d, 0x00, 0x20, 0x35, 0x80, 0x72, 0xd6, 0x36, 0x58, 0x80, 0xd1, 0xae, 0xea,
        0x32, 0x9a, 0xdf, 0x91, 0x21, 0x38, 0x38, 0x51, 0xed, 0x21, 0xa2, 0x8e, 0x3b, 0x75, 0xe9, 0x65, 0xd0, 0xd2, 0xcd, 0x16, 0x62, 0x54,
        0x00, 0x2d, 0x00, 0x02, 0x01, 0x01, 0x00, 0x2b, 0x00, 0x03, 0x02, 0x03, 0x04, 0x00, 0x39, 0x00, 0x31, 0x03, 0x04, 0x80, 0x00, 0xff,
        0xf7, 0x04, 0x04, 0x80, 0xa0, 0x00, 0x00, 0x05, 0x04, 0x80, 0x10, 0x00, 0x00, 0x06, 0x04, 0x80, 0x10, 0x00, 0x00, 0x07, 0x04, 0x80,
        0x10, 0x00, 0x00, 0x08, 0x01, 0x0a, 0x09, 0x01, 0x0a, 0x0a, 0x01, 0x03, 0x0b, 0x01, 0x19, 0x0f, 0x05, 0x63, 0x5f, 0x63, 0x69, 0x64
    };

    EncryptionKey testClientKey = EncryptionKey::newInitial(test_dcid_iovec, "client in");
    std::vector<uint8_t> testClientFinalContents = protectPacket(testClientUnprotectedData, 0, 23, 1, testClientKey);


    std::cout << "test client unprotected data size: " << testClientUnprotectedData.size() << std::endl;
    for (int i = 0; i < testClientUnprotectedData.size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)testClientUnprotectedData[i] << " ";
    }
    std::cout << std::dec << std::endl;

    std::cout << "test client final contents size: " << testClientFinalContents.size() << std::endl;
    for (int i = 0; i < testClientFinalContents.size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)testClientFinalContents[i] << " ";
    }
    std::cout << std::dec << std::endl;

    std::vector <uint8_t> expectedClientProtectedData = {
        0xcd, // protected header byte
        0x00, 0x00, 0x00, 0x01, // version
        0x08, // dcid len
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // dcid
        0x05, // scid len
        0x63, 0x5f, 0x63, 0x69, 0x64, 0x00, // scid
        0x41, 0x03, // packet length
        0x98, // protected packet number
        // encrypted crypto frame
        0x1c, 0x36, 0xa7, 0xed, 0x78, 0x71, 0x6b, 0xe9, 0x71, 0x1b, 0xa4, 0x98, 0xb7, 0xed, 0x86, 0x84, 0x43, 0xbb, 0x2e, 0x0c, 0x51, 0x4d,
        0x4d, 0x84, 0x8e, 0xad, 0xcc, 0x7a, 0x00, 0xd2, 0x5c, 0xe9, 0xf9, 0xaf, 0xa4, 0x83, 0x97, 0x80, 0x88, 0xde, 0x83, 0x6b, 0xe6, 0x8c,
        0x0b, 0x32, 0xa2, 0x45, 0x95, 0xd7, 0x81, 0x3e, 0xa5, 0x41, 0x4a, 0x91, 0x99, 0x32, 0x9a, 0x6d, 0x9f, 0x7f, 0x76, 0x0d, 0xd8, 0xbb,
        0x24, 0x9b, 0xf3, 0xf5, 0x3d, 0x9a, 0x77, 0xfb, 0xb7, 0xb3, 0x95, 0xb8, 0xd6, 0x6d, 0x78, 0x79, 0xa5, 0x1f, 0xe5, 0x9e, 0xf9, 0x60,
        0x1f, 0x79, 0x99, 0x8e, 0xb3, 0x56, 0x8e, 0x1f, 0xdc, 0x78, 0x9f, 0x64, 0x0a, 0xca, 0xb3, 0x85, 0x8a, 0x82, 0xef, 0x29, 0x30, 0xfa,
        0x5c, 0xe1, 0x4b, 0x5b, 0x9e, 0xa0, 0xbd, 0xb2, 0x9f, 0x45, 0x72, 0xda, 0x85, 0xaa, 0x3d, 0xef, 0x39, 0xb7, 0xef, 0xaf, 0xff, 0xa0,
        0x74, 0xb9, 0x26, 0x70, 0x70, 0xd5, 0x0b, 0x5d, 0x07, 0x84, 0x2e, 0x49, 0xbb, 0xa3, 0xbc, 0x78, 0x7f, 0xf2, 0x95, 0xd6, 0xae, 0x3b,
        0x51, 0x43, 0x05, 0xf1, 0x02, 0xaf, 0xe5, 0xa0, 0x47, 0xb3, 0xfb, 0x4c, 0x99, 0xeb, 0x92, 0xa2, 0x74, 0xd2, 0x44, 0xd6, 0x04, 0x92,
        0xc0, 0xe2, 0xe6, 0xe2, 0x12, 0xce, 0xf0, 0xf9, 0xe3, 0xf6, 0x2e, 0xfd, 0x09, 0x55, 0xe7, 0x1c, 0x76, 0x8a, 0xa6, 0xbb, 0x3c, 0xd8,
        0x0b, 0xbb, 0x37, 0x55, 0xc8, 0xb7, 0xeb, 0xee, 0x32, 0x71, 0x2f, 0x40, 0xf2, 0x24, 0x51, 0x19, 0x48, 0x70, 0x21, 0xb4, 0xb8, 0x4e,
        0x15, 0x65, 0xe3, 0xca, 0x31, 0x96, 0x7a, 0xc8, 0x60, 0x4d, 0x40, 0x32, 0x17, 0x0d, 0xec, 0x28, 0x0a, 0xee, 0xfa, 0x09, 0x5d, 0x08,
        // auth tag
        0xb3, 0xb7, 0x24, 0x1e, 0xf6, 0x64, 0x6a, 0x6c, 0x86, 0xe5, 0xc6, 0x2c, 0xe0, 0x8b, 0xe0, 0x99
    };

    ASSERT(testClientFinalContents.size() == expectedClientProtectedData.size());
    for (int i = 0; i < expectedClientProtectedData.size(); i++) {
        ASSERT(expectedClientProtectedData[i] == testClientFinalContents[i]);
    }





    // self test from https://quic.xargs.org/#server-initial-packet

    std::vector<uint8_t> testUnprotectedData = {
        0xc0, // unprotected header byte
        0x00, 0x00, 0x00, 0x01, // version
        0x05, // dcid len
        0x63, 0x5f, 0x63, 0x69, 0x64, // dcid
        0x05, // scid len
        0x73, 0x5f, 0x63, 0x69, 0x64, // scid
        0x00, // token
        0x40, 0x75, // packet length
        0x00, // unprotected packet number

        // ack frame
        0x02, 0x00, 0x42, 0x40, 0x00, 0x00,
        // crypto frame
        0x06, 0x00, 0x40, 0x5a, 0x02, 0x00, 0x00, 0x56, 0x03, 0x03, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82,
        0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x00, 0x13, 0x01, 0x00, 0x00, 0x2e, 0x00, 0x33, 0x00, 0x24, 0x00, 0x1d, 0x00, 0x20, 0x9f, 0xd7,
        0xad, 0x6d, 0xcf, 0xf4, 0x29, 0x8d, 0xd3, 0xf9, 0x6d, 0x5b, 0x1b, 0x2a, 0xf9, 0x10, 0xa0, 0x53, 0x5b, 0x14, 0x88, 0xd7, 0xf8, 0xfa, 0xbb, 0x34, 0x9a, 0x98, 0x28, 0x80, 0xb6, 0x15,
        0x00, 0x2b, 0x00, 0x02, 0x03, 0x04
    };

    EncryptionKey testServerKey = EncryptionKey::newInitial(test_dcid_iovec, "server in");
    std::vector<uint8_t> testFinalContents = protectPacket(testUnprotectedData, 0, 20, 1, testServerKey);


    std::cout << "test unprotected data size: " << testUnprotectedData.size() << std::endl;
    for (int i = 0; i < testUnprotectedData.size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)testUnprotectedData[i] << " ";
    }
    std::cout << std::dec << std::endl;

    std::cout << "test final contents size: " << testFinalContents.size() << std::endl;
    for (int i = 0; i < testFinalContents.size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)testFinalContents[i] << " ";
    }
    std::cout << std::dec << std::endl;

    std::vector <uint8_t> expectedProtectedData = {

        0xcd,  // protected header byte
        0x00, 0x00, 0x00, 0x01, // version
        0x05, // dcid len
        0x63, 0x5f, 0x63, 0x69, 0x64, // dcid
        0x05, // scid len
        0x73, 0x5f, 0x63, 0x69, 0x64, // scid
        0x00, // token
        0x40, 0x75, // packet length
        0x3a, // protected packet number

        // encrypted ack and crypto frames

        0x83, 0x68, 0x55, 0xd5, 0xd9, 0xc8, 0x23, 0xd0, 0x7c, 0x61, 0x68, 0x82, 0xca, 0x77, 0x02, 0x79, 0x24, 0x98, 0x64, 0xb5, 0x56,
        0xe5, 0x16, 0x32, 0x25, 0x7e, 0x2d, 0x8a, 0xb1, 0xfd, 0x0d, 0xc0, 0x4b, 0x18, 0xb9, 0x20, 0x3f, 0xb9, 0x19, 0xd8, 0xef, 0x5a, 0x33,
        0xf3, 0x78, 0xa6, 0x27, 0xdb, 0x67, 0x4d, 0x3c, 0x7f, 0xce, 0x6c, 0xa5, 0xbb, 0x3e, 0x8c, 0xf9, 0x01, 0x09, 0xcb, 0xb9, 0x55, 0x66,
        0x5f, 0xc1, 0xa4, 0xb9, 0x3d, 0x05, 0xf6, 0xeb, 0x83, 0x25, 0x2f, 0x66, 0x31, 0xbc, 0xad, 0xc7, 0x40, 0x2c, 0x10, 0xf6, 0x5c, 0x52,
        0xed, 0x15, 0xb4, 0x42, 0x9c, 0x9f, 0x64, 0xd8, 0x4d, 0x64, 0xfa, 0x40, 0x6c,
        // auth tag
        0xf0, 0xb5, 0x17, 0xa9, 0x26, 0xd6, 0x2a, 0x54, 0xa9, 0x29, 0x41, 0x36, 0xb1, 0x43, 0xb0, 0x33

    };

    ASSERT(testFinalContents.size() == expectedProtectedData.size());
    for (int i = 0; i < expectedProtectedData.size(); i++) {
        ASSERT(expectedProtectedData[i] == testFinalContents[i]);
    }

}

const Ptr<Chunk> EncryptedQuicPacketSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    ASSERT(false);
    return nullptr;
}


} // namespace quic
} // namespace inet
