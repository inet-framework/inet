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

#include "inet/transportlayer/quic/packet/QuicFrameSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/Endian.h"

namespace inet {
namespace quic {

Register_Serializer(FrameHeader, QuicFrameSerializer);
Register_Serializer(StreamFrameHeader, QuicFrameSerializer);
Register_Serializer(AckFrameHeader, QuicFrameSerializer);
Register_Serializer(MaxDataFrameHeader, QuicFrameSerializer);
Register_Serializer(MaxStreamDataFrameHeader, QuicFrameSerializer);
Register_Serializer(DataBlockedFrameHeader, QuicFrameSerializer);
Register_Serializer(StreamDataBlockedFrameHeader, QuicFrameSerializer);
Register_Serializer(PingFrameHeader, QuicFrameSerializer);
Register_Serializer(PaddingFrameHeader, QuicFrameSerializer);
Register_Serializer(CryptoFrameHeader, QuicFrameSerializer);
Register_Serializer(HandshakeDoneFrameHeader, QuicFrameSerializer);
Register_Serializer(ConnectionCloseFrameHeader, QuicFrameSerializer);

void QuicFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& frame = staticPtrCast<const FrameHeader>(chunk);
    FrameHeaderType frameType = frame->getFrameType();

    switch (frameType) {
        case FRAME_HEADER_TYPE_STREAM:
            serializeStreamFrame(stream, staticPtrCast<const StreamFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_ACK:
            serializeAckFrame(stream, staticPtrCast<const AckFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_MAX_DATA:
            serializeMaxDataFrame(stream, staticPtrCast<const MaxDataFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_MAX_STREAM_DATA:
            serializeMaxStreamDataFrame(stream, staticPtrCast<const MaxStreamDataFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_DATA_BLOCKED:
            serializeDataBlockedFrame(stream, staticPtrCast<const DataBlockedFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED:
            serializeStreamDataBlockedFrame(stream, staticPtrCast<const StreamDataBlockedFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_PING:
            serializePingFrame(stream, staticPtrCast<const PingFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_PADDING:
            serializePaddingFrame(stream, staticPtrCast<const PaddingFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_CRYPTO:
            serializeCryptoFrame(stream, staticPtrCast<const CryptoFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_HANDSHAKE_DONE:
            serializeHandshakeDoneFrame(stream, staticPtrCast<const HandshakeDoneFrameHeader>(frame));
            break;
        case FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC:
        case FRAME_HEADER_TYPE_CONNECTION_CLOSE_APP:
            serializeConnectionCloseFrame(stream, staticPtrCast<const ConnectionCloseFrameHeader>(frame));
            break;
        default:
            throw cRuntimeError("Unknown frame type: %d", frameType);
    }
}

void QuicFrameSerializer::serializeStreamFrame(MemoryOutputStream& stream, const Ptr<const StreamFrameHeader>& frame) const
{
    // Frame Type with FIN bit
    uint8_t frameType = frame->getFrameType();
    if (frame->getFinBit()) {
        frameType |= 0x01; // Set FIN bit
    }
    stream.writeByte(frameType);

    // Stream ID
    serializeVariableLengthInteger(stream, frame->getStreamId());

    // Offset
    serializeVariableLengthInteger(stream, frame->getOffset());

    // Length
    serializeVariableLengthInteger(stream, frame->getLength());
}

void QuicFrameSerializer::serializeAckFrame(MemoryOutputStream& stream, const Ptr<const AckFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());

    // Largest Acknowledged
    serializeVariableLengthInteger(stream, frame->getLargestAck());

    // ACK Delay
    serializeVariableLengthInteger(stream, frame->getAckDelay());

    // ACK Range Count
    serializeVariableLengthInteger(stream, frame->getAckRangeCount());

    // First ACK Range
    serializeVariableLengthInteger(stream, frame->getFirstAckRange());

    // ACK Ranges
    for (size_t i = 0; i < frame->getAckRangeArraySize(); i++) {
        const AckRange& ackRange = frame->getAckRange(i);
        serializeVariableLengthInteger(stream, ackRange.gap);
        serializeVariableLengthInteger(stream, ackRange.ackRange);
    }
}

void QuicFrameSerializer::serializeMaxDataFrame(MemoryOutputStream& stream, const Ptr<const MaxDataFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());

    // Maximum Data
    serializeVariableLengthInteger(stream, frame->getMaximumData());
}

void QuicFrameSerializer::serializeMaxStreamDataFrame(MemoryOutputStream& stream, const Ptr<const MaxStreamDataFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());

    // Stream ID
    serializeVariableLengthInteger(stream, frame->getStreamId());

    // Maximum Stream Data
    serializeVariableLengthInteger(stream, frame->getMaximumStreamData());
}

void QuicFrameSerializer::serializeDataBlockedFrame(MemoryOutputStream& stream, const Ptr<const DataBlockedFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());

    // Data Limit
    serializeVariableLengthInteger(stream, frame->getDataLimit());
}

void QuicFrameSerializer::serializeStreamDataBlockedFrame(MemoryOutputStream& stream, const Ptr<const StreamDataBlockedFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());

    // Stream ID
    serializeVariableLengthInteger(stream, frame->getStreamId());

    // Stream Data Limit
    serializeVariableLengthInteger(stream, frame->getStreamDataLimit());
}

void QuicFrameSerializer::serializePingFrame(MemoryOutputStream& stream, const Ptr<const PingFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());
}

void QuicFrameSerializer::serializePaddingFrame(MemoryOutputStream& stream, const Ptr<const PaddingFrameHeader>& frame) const
{
    stream.writeByteRepeatedly(0, B(frame->getChunkLength()).get());
}

void QuicFrameSerializer::serializeCryptoFrame(MemoryOutputStream& stream, const Ptr<const CryptoFrameHeader>& frame) const
{
    std::cout << "serializeCryptoFrame begin, stream length: " << stream.getLength() << std::endl;

    // Frame Type
    stream.writeByte(frame->getFrameType());

    // Offset
    serializeVariableLengthInteger(stream, frame->getOffset());

    // Length
    serializeVariableLengthInteger(stream, frame->getLength());

    std::cout << "serializeCryptoFrame end, stream length: " << stream.getLength() << std::endl;
}

void QuicFrameSerializer::serializeHandshakeDoneFrame(MemoryOutputStream& stream, const Ptr<const HandshakeDoneFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());
}

void QuicFrameSerializer::serializeConnectionCloseFrame(MemoryOutputStream& stream, const Ptr<const ConnectionCloseFrameHeader>& frame) const
{
    // Frame Type
    stream.writeByte(frame->getFrameType());

    // Error Code
    serializeVariableLengthInteger(stream, frame->getErrorCode());
}

void QuicFrameSerializer::serializeTransportParametersExtension(MemoryOutputStream& stream, const Ptr<const TransportParametersExtension>& frame) const
{
    // Initial Max Data
    serializeVariableLengthInteger(stream, frame->getInitialMaxData());

    // Initial Max Stream Data Bidi Local
    serializeVariableLengthInteger(stream, frame->getInitialMaxStreamDataBidiLocal());

    // Initial Max Stream Data Bidi Remote
    serializeVariableLengthInteger(stream, frame->getInitialMaxStreamDataBidiRemote());

    // Initial Max Stream Data Uni
    serializeVariableLengthInteger(stream, frame->getInitialMaxStreamDataUni());
}

const Ptr<Chunk> QuicFrameSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t frameType = stream.readByte();
    return deserializeFrame(stream, frameType);
}

const Ptr<Chunk> QuicFrameSerializer::deserializeFrame(MemoryInputStream& stream, uint8_t frameType) const
{
    // Check for STREAM frames (which have bits 4-7 set to 0x10xx)
    if ((frameType & 0xF0) == 0x10) {
        return deserializeStreamFrame(stream, frameType);
    }

    // For other frame types, use a switch
    switch (frameType) {
        case FRAME_HEADER_TYPE_ACK:
            return deserializeAckFrame(stream);
        case FRAME_HEADER_TYPE_MAX_DATA:
            return deserializeMaxDataFrame(stream);
        case FRAME_HEADER_TYPE_MAX_STREAM_DATA:
            return deserializeMaxStreamDataFrame(stream);
        case FRAME_HEADER_TYPE_DATA_BLOCKED:
            return deserializeDataBlockedFrame(stream);
        case FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED:
            return deserializeStreamDataBlockedFrame(stream);
        case FRAME_HEADER_TYPE_PING:
            return deserializePingFrame(stream);
        case FRAME_HEADER_TYPE_PADDING:
            return deserializePaddingFrame(stream);
        case FRAME_HEADER_TYPE_CRYPTO:
            return deserializeCryptoFrame(stream);
        case FRAME_HEADER_TYPE_HANDSHAKE_DONE:
            return deserializeHandshakeDoneFrame(stream);
        case FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC:
        case FRAME_HEADER_TYPE_CONNECTION_CLOSE_APP:
            return deserializeConnectionCloseFrame(stream, frameType);
        default:
            throw cRuntimeError("Unknown frame type: %d", frameType);
    }
}

const Ptr<Chunk> QuicFrameSerializer::deserializeStreamFrame(MemoryInputStream& stream, uint8_t frameType) const
{
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

const Ptr<Chunk> QuicFrameSerializer::deserializeAckFrame(MemoryInputStream& stream) const
{
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

const Ptr<Chunk> QuicFrameSerializer::deserializeMaxDataFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<MaxDataFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_MAX_DATA);

    // Maximum Data
    header->setMaximumData(deserializeVariableLengthInteger(stream));

    header->calcChunkLength();
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializeMaxStreamDataFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<MaxStreamDataFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_MAX_STREAM_DATA);

    // Stream ID
    header->setStreamId(deserializeVariableLengthInteger(stream));

    // Maximum Stream Data
    header->setMaximumStreamData(deserializeVariableLengthInteger(stream));

    header->calcChunkLength();
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializeDataBlockedFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<DataBlockedFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_DATA_BLOCKED);

    // Data Limit
    header->setDataLimit(deserializeVariableLengthInteger(stream));

    header->calcChunkLength();
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializeStreamDataBlockedFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<StreamDataBlockedFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED);

    // Stream ID
    header->setStreamId(deserializeVariableLengthInteger(stream));

    // Stream Data Limit
    header->setStreamDataLimit(deserializeVariableLengthInteger(stream));

    header->calcChunkLength();
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializePingFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<PingFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_PING);
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializePaddingFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<PaddingFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_PADDING);
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializeCryptoFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<CryptoFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_CRYPTO);

    // Offset
    header->setOffset(deserializeVariableLengthInteger(stream));

    // Length
    header->setLength(deserializeVariableLengthInteger(stream));

    header->calcChunkLength();
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializeHandshakeDoneFrame(MemoryInputStream& stream) const
{
    auto header = makeShared<HandshakeDoneFrameHeader>();
    header->setFrameType(FRAME_HEADER_TYPE_HANDSHAKE_DONE);
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializeConnectionCloseFrame(MemoryInputStream& stream, uint8_t frameType) const
{
    auto header = makeShared<ConnectionCloseFrameHeader>();
    header->setFrameType(static_cast<FrameHeaderType>(frameType));

    // Error Code
    header->setErrorCode(deserializeVariableLengthInteger(stream));

    header->calcChunkLength();
    return header;
}

const Ptr<Chunk> QuicFrameSerializer::deserializeTransportParametersExtension(MemoryInputStream& stream) const
{
    auto extension = makeShared<TransportParametersExtension>();

    // Initial Max Data
    extension->setInitialMaxData(deserializeVariableLengthInteger(stream));

    // Initial Max Stream Data Bidi Local
    extension->setInitialMaxStreamDataBidiLocal(deserializeVariableLengthInteger(stream));

    // Initial Max Stream Data Bidi Remote
    extension->setInitialMaxStreamDataBidiRemote(deserializeVariableLengthInteger(stream));

    // Initial Max Stream Data Uni
    extension->setInitialMaxStreamDataUni(deserializeVariableLengthInteger(stream));

    extension->calcChunkLength();
    return extension;
}

void QuicFrameSerializer::serializeVariableLengthInteger(MemoryOutputStream& stream, uint64_t value) const
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

uint64_t QuicFrameSerializer::deserializeVariableLengthInteger(MemoryInputStream& stream) const
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

} // namespace quic
} // namespace inet
