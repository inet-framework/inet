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

import inet.common.packet.chunk.Chunk;
import inet.transportlayer.quic.packet.VariableLengthInteger;

cplusplus{{
    #include "VariableLengthInteger.h"
}}

namespace inet::quic;

enum FrameHeaderType {
    FRAME_HEADER_TYPE_PADDING = 0x00;
    FRAME_HEADER_TYPE_PING = 0x01;
    FRAME_HEADER_TYPE_STREAM = 0x0e;
    FRAME_HEADER_TYPE_ACK = 0x02;
    FRAME_HEADER_TYPE_CRYPTO = 0x06;
    FRAME_HEADER_TYPE_MAX_DATA = 0x10;
    FRAME_HEADER_TYPE_MAX_STREAM_DATA = 0x11;   
    FRAME_HEADER_TYPE_DATA_BLOCKED = 0x14;   
    FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED = 0x15;
    FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC = 0x1c;
    FRAME_HEADER_TYPE_CONNECTION_CLOSE_APP = 0x1d;
    FRAME_HEADER_TYPE_HANDSHAKE_DONE = 0x1e;
}

class FrameHeader extends FieldsChunk
{
    FrameHeaderType frameType;
}

class StreamFrameHeader extends FrameHeader
{
    frameType = FRAME_HEADER_TYPE_STREAM;
    bool finBit; // not a real field. Draft encodes this bit into type field.
    VariableLengthInteger streamId;
    VariableLengthInteger offset;
    VariableLengthInteger length;
}
cplusplus(StreamFrameHeader) {{
  public:
    static const size_t MAX_HEADER_SIZE = 1 + 8 + 8 + 8;
    static const size_t MIN_HEADER_SIZE = 1 + 1 + 1 + 1;

    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(streamId) + getVariableLengthIntegerSize(offset) + getVariableLengthIntegerSize(length);
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "STREAM[streamId=" << streamId
            << ", offset=" << offset
            << ", length=" << length
            << "]";
        return os.str();
    }
}}

struct AckRange
{
    VariableLengthInteger gap;
    VariableLengthInteger ackRange;
};

class AckFrameHeader extends FrameHeader
{
    frameType = FRAME_HEADER_TYPE_ACK;
    VariableLengthInteger largestAck;
    VariableLengthInteger ackDelay;
    VariableLengthInteger ackRangeCount;
    VariableLengthInteger firstAckRange;
    AckRange ackRange[];
    //uint64_t ecnCounts;
    //uint64_t ecnCounts[];
}
cplusplus(AckFrameHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(largestAck) + getVariableLengthIntegerSize(ackDelay) + getVariableLengthIntegerSize(ackRangeCount) + getVariableLengthIntegerSize(firstAckRange);
        for (int i=0; i<getAckRangeArraySize(); i++) {
            const AckRange& ackRangeItem = getAckRange(i);
            len += getVariableLengthIntegerSize(ackRangeItem.gap);
            len += getVariableLengthIntegerSize(ackRangeItem.ackRange);
        }
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "ACK[largest_ack=" << largestAck
            << ", ack_delay=" << ackDelay
            << ", ack_range_count=" << ackRangeCount
            << ", first_ack_range=" << firstAckRange;

		for (int i=0; i<getAckRangeArraySize(); i++) {
            const AckRange& ackRangeItem = getAckRange(i);
            os << ", (gap=" << ackRangeItem.gap
                << ", ack_range=" << ackRangeItem.ackRange
                << ")";
        }
        os << "]";
        return os.str();
    }
}}

class MaxDataFrameHeader extends FrameHeader
{
	frameType = FRAME_HEADER_TYPE_MAX_DATA;
	VariableLengthInteger maximumData;
} 
cplusplus(MaxDataFrameHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(maximumData);
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "MAX_DATA[maximumData=" << maximumData << "]";
        return os.str();
    }
}}

class MaxStreamDataFrameHeader extends FrameHeader
{
	frameType = FRAME_HEADER_TYPE_MAX_STREAM_DATA;	
	VariableLengthInteger streamId;
	VariableLengthInteger maximumStreamData;
}    

cplusplus(MaxStreamDataFrameHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(streamId) + getVariableLengthIntegerSize(maximumStreamData);
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "MAX_STREAM_DATA[streamId=" << streamId << ", maximumStreamData=" << maximumStreamData << "]";
        return os.str();
    }
}}

class DataBlockedFrameHeader extends FrameHeader
{
	frameType = FRAME_HEADER_TYPE_DATA_BLOCKED;	
	VariableLengthInteger dataLimit;
}    

cplusplus(DataBlockedFrameHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(dataLimit);
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "DATA_BLOCKED[dataLimit=" << dataLimit << "]";
        return os.str();
    }
}}

class StreamDataBlockedFrameHeader extends FrameHeader
{
	frameType = FRAME_HEADER_TYPE_STREAM_DATA_BLOCKED;	
	VariableLengthInteger streamId;
	VariableLengthInteger streamDataLimit;
}    

cplusplus(StreamDataBlockedFrameHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(streamId) + getVariableLengthIntegerSize(streamDataLimit);
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "STREAM_DATA_BLOCKED[streamId=" << streamId << ", streamDataLimit=" << streamDataLimit << "]";
        return os.str();
    }
}}

class PingFrameHeader extends FrameHeader
{
    chunkLength = B(1);
    frameType = FRAME_HEADER_TYPE_PING;
}

class PaddingFrameHeader extends FrameHeader
{
    chunkLength = B(1);
    frameType = FRAME_HEADER_TYPE_PADDING;
}
cplusplus(PaddingFrameHeader) {{
  public:
    void setLength(int lengthInByte) {
        this->setChunkLength(B(lengthInByte));
    }
}}

class CryptoFrameHeader extends FrameHeader
{
    frameType = FRAME_HEADER_TYPE_CRYPTO;
    VariableLengthInteger offset;
    VariableLengthInteger length;
}
cplusplus(CryptoFrameHeader) {{
  public:
    static const size_t MAX_HEADER_SIZE = 1 + 8 + 8;
    static const size_t MIN_HEADER_SIZE = 1 + 1 + 1;

    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(offset) + getVariableLengthIntegerSize(length);
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "CRYPTO[offset=" << offset
            << ", length=" << length
            << "]";
        return os.str();
    }
}}

class HandshakeDoneFrameHeader extends FrameHeader
{
    chunkLength = B(1);
    frameType = FRAME_HEADER_TYPE_HANDSHAKE_DONE;
}

class ConnectionCloseFrameHeader extends FrameHeader
{
    frameType = FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC;
    VariableLengthInteger errorCode;
}
cplusplus(ConnectionCloseFrameHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + getVariableLengthIntegerSize(errorCode);
        this->setChunkLength(B(len));
    }
    std::string str() const override {
        std::ostringstream os;
        os << "CONNECTION_CLOSE["
            << (frameType == FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC ? "QUIC" : "APP")
            << ", errorCode=" << errorCode
            << "]";
        return os.str();
    }
}}

