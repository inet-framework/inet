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

#include "inet/transportlayer/quic/packet/EncryptedQuicPacketChunk.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/Endian.h"


extern "C" {
#include "picotls.h"
#include "picotls/openssl.h"
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
        size_t packetNumberLength = 0; // TODO set for other kinds of packet too
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
                    packetNumberLength = initialPacketHeader->getPacketNumberLength();
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

        std::vector<uint8_t> unencryptedData = stream.getData();
        stream.clear();

        ptls_cipher_suite_t *cs = &ptls_openssl_aes128gcmsha256;


        static const uint8_t quic_v1_salt[] = {
            0x38, 0x76, 0x2c, 0xf7, 0xf5, 0x59, 0x34, 0xb3,
            0x4d, 0x17, 0x9a, 0xe6, 0xa4, 0xc8, 0x0c, 0xad,
            0xcc, 0xbb, 0x7f, 0x0a
        };
        uint8_t master_secret[PTLS_MAX_DIGEST_SIZE];



        uint8_t dcid = longHeader->getDstConnectionId();

        ptls_hkdf_extract(cs->hash,
                master_secret, ptls_iovec_init(quic_v1_salt, sizeof(quic_v1_salt)), ptls_iovec_init(&dcid, sizeof(dcid)));

        //int ptls_hkdf_expand_label(ptls_hash_algorithm_t *algo, void *output, size_t outlen, ptls_iovec_t secret, const char *label,
        //                           ptls_iovec_t hash_value, const char *label_prefix);

        uint8_t client_secret[PTLS_MAX_DIGEST_SIZE];
        ptls_hkdf_expand_label(cs->hash, client_secret, cs->aead->ctr_cipher->key_size, ptls_iovec_init(master_secret, cs->hash->digest_size),
                                              "client in", ptls_iovec_init(NULL, 0), NULL);


        uint8_t hpkey[PTLS_MAX_SECRET_SIZE];


        ptls_cipher_context_t *header_protect;
        ptls_aead_context_t *packet_protect;



        ptls_hkdf_expand_label(cs->hash, hpkey, cs->aead->ctr_cipher->key_size, ptls_iovec_init(client_secret, cs->hash->digest_size),
                                              "quic hp", ptls_iovec_init(NULL, 0), NULL);

        header_protect = ptls_cipher_new(cs->aead->ctr_cipher, 1, hpkey);


        // generate new AEAD context
        packet_protect = ptls_aead_new(cs->aead, cs->hash, 1, client_secret, "quic "); // quicly uses a different label prefix for some reason

/*

        engine->encrypt_packet(engine, NULL, header_protect, packet_protect, ptls_iovec_init(buf, packet_size), 0,
                               pn_off + QUICLY_SEND_PN_SIZE, buf[pn_off] * 256 + buf[pn_off + 1], 0);


static void default_finalize_send_packet(quicly_crypto_engine_t *engine, quicly_conn_t *conn,
                                         ptls_cipher_context_t *header_protect_ctx, ptls_aead_context_t *packet_protect_ctx,
                                         ptls_iovec_t datagram, size_t first_byte_at, size_t payload_from, uint64_t packet_number,
                                         int coalesced)

    */

        //uint8_t buf[1500] = {}, secret[PTLS_MAX_DIGEST_SIZE];
        //size_t inlen, pn_off, packet_size;

        size_t payload_from = B(longHeader->getChunkLength()).get();
        ptls_aead_supplementary_encryption_t supp = {.ctx = header_protect,
                                                     .input = unencryptedData.data() + payload_from - packetNumberLength + 4};


        std::vector<uint8_t> encryptedPayload;
        encryptedPayload.resize(unencryptedData.size() - payload_from + packet_protect->algo->tag_size);

        size_t packetNumber = 0; // correct for initial packet, TODO otherwise

        //  inline void ptls_aead_encrypt_s(ptls_aead_context_t *ctx, void *output, const void *input, size_t inlen, uint64_t seq,
        //                                const void *aad, size_t aadlen, ptls_aead_supplementary_encryption_t *supp)
        ptls_aead_encrypt_s(packet_protect, encryptedPayload.data(), unencryptedData.data() + payload_from,
                                       unencryptedData.size() - payload_from,
                                       packetNumber,
                                       unencryptedData.data(), payload_from, &supp);

        unencryptedData[0] ^= supp.output[0] & 0xf;
        for (size_t i = 0; i != packetNumberLength; ++i)
            unencryptedData[payload_from + i - packetNumberLength] ^= supp.output[i + 1];

        stream.writeBytes(unencryptedData.data(), B(payload_from));
        // TODO also write the auth tag after contents
        stream.writeBytes(encryptedPayload.data(), B(encryptedPayload.size()    /* should not subtract this  */  - packet_protect->algo->tag_size));
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

void QuicPacketHeaderSerializer::serializeLongPacketHeader(MemoryOutputStream& stream, const Ptr<const LongPacketHeader>& header, uint8_t packetNumberLength) const
{
    std::cout << "serializeLongPacketHeader begin, stream length: " << stream.getLength() << std::endl;
    // First byte: Header form (1 bit) | Reserved (1 bit) | Packet type (2 bits) | Type-specific bits (4 bits)
    uint8_t firstByte = (header->getHeaderForm() << 7) | (1 << 6) |  (header->getLongPacketType() << 4) | (packetNumberLength-1);
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
    // TODO: assuming this is set to 1 for initial packets
    serializeLongPacketHeader(stream, header, header->getPacketNumberLength());

    // Token Length
    serializeVariableLengthInteger(stream, header->getTokenLength());

    // Token (if present)
    if (header->getTokenLength() > 0) {
        stream.writeUint64Be(header->getToken());
    }

    // Length
    serializeVariableLengthInteger(stream, header->getLength());

    // Packet Number
    stream.writeByte(0); // initial packet number is 1
    std::cout << "serializeInitialPacketHeader end, stream length: " << stream.getLength() << std::endl;
}

void QuicPacketHeaderSerializer::serializeHandshakePacketHeader(MemoryOutputStream& stream, const Ptr<const HandshakePacketHeader>& header) const
{
    // Serialize the common long header fields
    serializeLongPacketHeader(stream, header, header->getPacketNumberLength());

    // Length
    serializeVariableLengthInteger(stream, header->getLength());

    // Packet Number
    stream.writeUint32Be(header->getPacketNumber());
}

void QuicPacketHeaderSerializer::serializeZeroRttPacketHeader(MemoryOutputStream& stream, const Ptr<const ZeroRttPacketHeader>& header) const
{
    // Serialize the common long header fields
    serializeLongPacketHeader(stream, header, header->getPacketNumberLength());

    // Length
    serializeVariableLengthInteger(stream, header->getLength());

    // Packet Number
    stream.writeUint32Be(header->getPacketNumber());
}

void QuicPacketHeaderSerializer::serializeRetryPacketHeader(MemoryOutputStream& stream, const Ptr<const RetryPacketHeader>& header) const
{
    // Serialize the common long header fields
    serializeLongPacketHeader(stream, header, 1); // TODO: packet number length?

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

void EncryptedQuicPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto payload = staticPtrCast<const EncryptedQuicPacketChunk>(chunk)->getChunk();
    b payloadLength = payload->getChunkLength();

    // whether any part of the payload is asked to be serialized
    if (offset < payloadLength) {
        const Chunk *payloadPointer = payload.get();
        auto serializer = ChunkSerializerRegistry::getInstance().getSerializer(typeid(*payloadPointer));
        b payloadToSerialize = std::min(length, payloadLength - offset);
        serializer->serialize(stream, payload, offset, payloadToSerialize);
    }

    // whether any part of the auth tag is asked to be serialized
    if (offset + length > payloadLength) {
        ASSERT(offset + length <= payloadLength + B(16));
        size_t authTagStartByteIndex = std::max((int64_t)0, B(offset - payloadLength).get());
        size_t authTagEndByteIndex = B(offset + length - payloadLength).get();
        for (int i = authTagStartByteIndex; i < authTagEndByteIndex; i++) {
            stream.writeByte((i << 4) + i);
        }
    }

}

const Ptr<Chunk> EncryptedQuicPacketSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{

}


} // namespace quic
} // namespace inet
