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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/Endian.h"

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

Register_Serializer(FrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(StreamFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(AckFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(MaxDataFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(MaxStreamDataFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(DataBlockedFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(StreamDataBlockedFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(PingFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(PaddingFrameHeader, QuicFrameHeaderSerializer);
Register_Serializer(CryptoFrameHeader, QuicFrameHeaderSerializer);

// Helper methods for serializing and deserializing variable length integers
void serializeVariableLengthInteger(MemoryOutputStream& stream, uint64_t value)
{
    if (value < 64) {
        // 0-6 bit length, 0 bits to identify length
        stream.writeByte(value);
    }
    else if (value < 16384) {
        // 7-14 bit length, 01 bits to identify length
        stream.writeByte(0x40 | (value >> 8));
        stream.writeByte(value & 0xFF);
    }
    else if (value < 1073741824) {
        // 15-30 bit length, 10 bits to identify length
        stream.writeByte(0x80 | (value >> 24));
        stream.writeByte((value >> 16) & 0xFF);
        stream.writeByte((value >> 8) & 0xFF);
        stream.writeByte(value & 0xFF);
    }
    else {
        // 31-62 bit length, 11 bits to identify length
        stream.writeByte(0xC0 | (value >> 56));
        stream.writeByte((value >> 48) & 0xFF);
        stream.writeByte((value >> 40) & 0xFF);
        stream.writeByte((value >> 32) & 0xFF);
        stream.writeByte((value >> 24) & 0xFF);
        stream.writeByte((value >> 16) & 0xFF);
        stream.writeByte((value >> 8) & 0xFF);
        stream.writeByte(value & 0xFF);
    }
}

uint64_t deserializeVariableLengthInteger(MemoryInputStream& stream)
{
    uint8_t firstByte = stream.readByte();
    uint8_t prefix = firstByte >> 6;
    uint64_t value = 0;

    switch (prefix) {
        case 0: // 0-6 bit length, 0 bits to identify length
            value = firstByte;
            break;
        case 1: // 7-14 bit length, 01 bits to identify length
            value = ((uint64_t)(firstByte & 0x3F) << 8) | stream.readByte();
            break;
        case 2: // 15-30 bit length, 10 bits to identify length
            value = ((uint64_t)(firstByte & 0x3F) << 24) |
                   ((uint64_t)stream.readByte() << 16) |
                   ((uint64_t)stream.readByte() << 8) |
                   stream.readByte();
            break;
        case 3: // 31-62 bit length, 11 bits to identify length
            value = ((uint64_t)(firstByte & 0x3F) << 56) |
                   ((uint64_t)stream.readByte() << 48) |
                   ((uint64_t)stream.readByte() << 40) |
                   ((uint64_t)stream.readByte() << 32) |
                   ((uint64_t)stream.readByte() << 24) |
                   ((uint64_t)stream.readByte() << 16) |
                   ((uint64_t)stream.readByte() << 8) |
                   stream.readByte();
            break;
    }

    return value;
}

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
                case LONG_PACKET_HEADER_TYPE_INITIAL:
                    serializeInitialPacketHeader(stream, staticPtrCast<const InitialPacketHeader>(longHeader));
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

void QuicPacketHeaderSerializer::serializeVariableLengthInteger(MemoryOutputStream& stream, uint64_t value) const
{
    ::inet::quic::serializeVariableLengthInteger(stream, value);
}

uint64_t QuicPacketHeaderSerializer::deserializeVariableLengthInteger(MemoryInputStream& stream) const
{
    return ::inet::quic::deserializeVariableLengthInteger(stream);
}

//
// QuicFrameHeaderSerializer
//

void QuicFrameHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const FrameHeader>(chunk);
    FrameHeaderType frameType = header->getFrameType();

    switch (frameType) {
        case FRAME_HEADER_TYPE_STREAM:
            serializeStreamFrameHeader(stream, staticPtrCast<const StreamFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_ACK:
            serializeAckFrameHeader(stream, staticPtrCast<const AckFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_MAX_DATA:
            serializeMaxDataFrameHeader(stream, staticPtrCast<const MaxDataFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_MAX_STREAM_DATA:
            serializeMaxStreamDataFrameHeader(stream, staticPtrCast<const MaxStreamDataFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_DATA_BLOCKED:
            serializeDataBlockedFrameHeader(stream, staticPtrCast<const DataBlockedFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED:
            serializeStreamDataBlockedFrameHeader(stream, staticPtrCast<const StreamDataBlockedFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_PING:
            serializePingFrameHeader(stream, staticPtrCast<const PingFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_PADDING:
            serializePaddingFrameHeader(stream, staticPtrCast<const PaddingFrameHeader>(header));
            break;
        case FRAME_HEADER_TYPE_CRYPTO:
            serializeCryptoFrameHeader(stream, staticPtrCast<const CryptoFrameHeader>(header));
            break;
        default:
            throw cRuntimeError("Unknown frame type: %d", frameType);
    }
}

void QuicFrameHeaderSerializer::serializeStreamFrameHeader(MemoryOutputStream& stream, const Ptr<const StreamFrameHeader>& header) const
{
    // Frame Type
    uint8_t frameType = header->getFrameType();
    if (header->getFinBit()) {
        frameType |= 0x01; // Set FIN bit
    }
    stream.writeByte(frameType);

    // Stream ID
    serializeVariableLengthInteger(stream, header->getStreamId());

    // Offset
    serializeVariableLengthInteger(stream, header->getOffset());

    // Length
    serializeVariableLengthInteger(stream, header->getLength());
}

void QuicFrameHeaderSerializer::serializeAckFrameHeader(MemoryOutputStream& stream, const Ptr<const AckFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());

    // Largest Acknowledged
    serializeVariableLengthInteger(stream, header->getLargestAck());

    // ACK Delay
    serializeVariableLengthInteger(stream, header->getAckDelay());

    // ACK Range Count
    serializeVariableLengthInteger(stream, header->getAckRangeCount());

    // First ACK Range
    serializeVariableLengthInteger(stream, header->getFirstAckRange());

    // ACK Ranges
    for (size_t i = 0; i < header->getAckRangeArraySize(); i++) {
        const AckRange& ackRange = header->getAckRange(i);
        serializeVariableLengthInteger(stream, ackRange.gap);
        serializeVariableLengthInteger(stream, ackRange.ackRange);
    }
}

void QuicFrameHeaderSerializer::serializeMaxDataFrameHeader(MemoryOutputStream& stream, const Ptr<const MaxDataFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());

    // Maximum Data
    serializeVariableLengthInteger(stream, header->getMaximumData());
}

void QuicFrameHeaderSerializer::serializeMaxStreamDataFrameHeader(MemoryOutputStream& stream, const Ptr<const MaxStreamDataFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());

    // Stream ID
    serializeVariableLengthInteger(stream, header->getStreamId());

    // Maximum Stream Data
    serializeVariableLengthInteger(stream, header->getMaximumStreamData());
}

void QuicFrameHeaderSerializer::serializeDataBlockedFrameHeader(MemoryOutputStream& stream, const Ptr<const DataBlockedFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());

    // Data Limit
    serializeVariableLengthInteger(stream, header->getDataLimit());
}

void QuicFrameHeaderSerializer::serializeStreamDataBlockedFrameHeader(MemoryOutputStream& stream, const Ptr<const StreamDataBlockedFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());

    // Stream ID
    serializeVariableLengthInteger(stream, header->getStreamId());

    // Stream Data Limit
    serializeVariableLengthInteger(stream, header->getStreamDataLimit());
}

void QuicFrameHeaderSerializer::serializePingFrameHeader(MemoryOutputStream& stream, const Ptr<const PingFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());
}

void QuicFrameHeaderSerializer::serializePaddingFrameHeader(MemoryOutputStream& stream, const Ptr<const PaddingFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());
}

void QuicFrameHeaderSerializer::serializeCryptoFrameHeader(MemoryOutputStream& stream, const Ptr<const CryptoFrameHeader>& header) const
{
    // Frame Type
    stream.writeByte(header->getFrameType());

    // Offset
    serializeVariableLengthInteger(stream, header->getOffset());

    // Length
    serializeVariableLengthInteger(stream, header->getLength());
}

const Ptr<Chunk> QuicFrameHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t frameTypeByte = stream.readByte();
    FrameHeaderType frameType = static_cast<FrameHeaderType>(frameTypeByte & 0xFE); // Clear the FIN bit for STREAM frames

    return deserializeFrameHeader(stream, frameType);
}

const Ptr<Chunk> QuicFrameHeaderSerializer::deserializeFrameHeader(MemoryInputStream& stream, FrameHeaderType frameType) const
{
    switch (frameType) {
        case FRAME_HEADER_TYPE_STREAM: {
            auto header = makeShared<StreamFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_STREAM);

            // FIN bit
            header->setFinBit((frameType & 0x01) != 0);

            // Stream ID
            header->setStreamId(deserializeVariableLengthInteger(stream));

            // Offset
            header->setOffset(deserializeVariableLengthInteger(stream));

            // Length
            header->setLength(deserializeVariableLengthInteger(stream));

            header->calcChunkLength();
            return header;
        }
        case FRAME_HEADER_TYPE_ACK: {
            auto header = makeShared<AckFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_ACK);

            // Largest Acknowledged
            header->setLargestAck(deserializeVariableLengthInteger(stream));

            // ACK Delay
            header->setAckDelay(deserializeVariableLengthInteger(stream));

            // ACK Range Count
            uint64_t ackRangeCount = deserializeVariableLengthInteger(stream);
            header->setAckRangeCount(ackRangeCount);

            // First ACK Range
            header->setFirstAckRange(deserializeVariableLengthInteger(stream));

            // ACK Ranges
            header->setAckRangeArraySize(ackRangeCount);
            for (size_t i = 0; i < ackRangeCount; i++) {
                AckRange ackRange;
                ackRange.gap = deserializeVariableLengthInteger(stream);
                ackRange.ackRange = deserializeVariableLengthInteger(stream);
                header->setAckRange(i, ackRange);
            }

            header->calcChunkLength();
            return header;
        }
        case FRAME_HEADER_TYPE_MAX_DATA: {
            auto header = makeShared<MaxDataFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_MAX_DATA);

            // Maximum Data
            header->setMaximumData(deserializeVariableLengthInteger(stream));

            header->calcChunkLength();
            return header;
        }
        case FRAME_HEADER_TYPE_MAX_STREAM_DATA: {
            auto header = makeShared<MaxStreamDataFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_MAX_STREAM_DATA);

            // Stream ID
            header->setStreamId(deserializeVariableLengthInteger(stream));

            // Maximum Stream Data
            header->setMaximumStreamData(deserializeVariableLengthInteger(stream));

            header->calcChunkLength();
            return header;
        }
        case FRAME_HEADER_TYPE_DATA_BLOCKED: {
            auto header = makeShared<DataBlockedFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_DATA_BLOCKED);

            // Data Limit
            header->setDataLimit(deserializeVariableLengthInteger(stream));

            header->calcChunkLength();
            return header;
        }
        case FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED: {
            auto header = makeShared<StreamDataBlockedFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED);

            // Stream ID
            header->setStreamId(deserializeVariableLengthInteger(stream));

            // Stream Data Limit
            header->setStreamDataLimit(deserializeVariableLengthInteger(stream));

            header->calcChunkLength();
            return header;
        }
        case FRAME_HEADER_TYPE_PING: {
            auto header = makeShared<PingFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_PING);
            return header;
        }
        case FRAME_HEADER_TYPE_PADDING: {
            auto header = makeShared<PaddingFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_PADDING);
            return header;
        }
        case FRAME_HEADER_TYPE_CRYPTO: {
            auto header = makeShared<CryptoFrameHeader>();
            header->setFrameType(FRAME_HEADER_TYPE_CRYPTO);

            // Offset
            header->setOffset(deserializeVariableLengthInteger(stream));

            // Length
            header->setLength(deserializeVariableLengthInteger(stream));

            header->calcChunkLength();
            return header;
        }
        default:
            throw cRuntimeError("Unknown frame type: %d", frameType);
    }
}

void QuicFrameHeaderSerializer::serializeVariableLengthInteger(MemoryOutputStream& stream, uint64_t value) const
{
    ::inet::quic::serializeVariableLengthInteger(stream, value);
}

uint64_t QuicFrameHeaderSerializer::deserializeVariableLengthInteger(MemoryInputStream& stream) const
{
    return ::inet::quic::deserializeVariableLengthInteger(stream);
}

} // namespace quic
} // namespace inet
