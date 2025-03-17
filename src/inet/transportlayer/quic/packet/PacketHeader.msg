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

enum PacketHeaderForm {
    PACKET_HEADER_FORM_SHORT = 0;
    PACKET_HEADER_FORM_LONG = 1;
}

enum LongPacketHeaderType {
    LONG_PACKET_HEADER_TYPE_INITIAL = 0x00;
    LONG_PACKET_HEADER_TYPE_0RTT = 0x01;
    LONG_PACKET_HEADER_TYPE_HANDSHAKE = 0x02;
    LONG_PACKET_HEADER_TYPE_RETRY = 0x03;
}

class PacketHeader extends FieldsChunk
{
    uint8_t headerForm;
}

class LongPacketHeader extends PacketHeader
{
    headerForm = PACKET_HEADER_FORM_LONG;
    uint8_t longPacketType;
    uint32_t version = 1;
    uint8_t dstConnectionIdLength;
    uint64_t dstConnectionId;
    uint8_t srcConnectionIdLength;
    uint64_t srcConnectionId;
}

class VersionNegotiationPacketHeader extends LongPacketHeader
{
    longPacketType = 0;
    version = 0;
    uint32_t supportedVersion[];
}

class InitialPacketHeader extends LongPacketHeader
{
    longPacketType = LONG_PACKET_HEADER_TYPE_INITIAL;
    uint8_t packetNumberLength;
    VariableLengthInteger tokenLength;
    uint64_t token;
    VariableLengthInteger length;
    uint32_t packetNumber;
    // payload
}
cplusplus(InitialPacketHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + 4 + 1 + dstConnectionIdLength + 1 + srcConnectionIdLength + getVariableLengthIntegerSize(length) + packetNumberLength;
        this->setChunkLength(B(len));
    }
       
    std::string str() const override {
        std::ostringstream os;
        os << "Initial[dstConnectionId=" << dstConnectionId
           << ", packetNumber=" << packetNumber
           << "]";
        return os.str();
    }
}}

class ZeroRttPacketHeader extends LongPacketHeader
{
	longPacketType = LONG_PACKET_HEADER_TYPE_0RTT;
	uint8_t packetNumberLength;
    VariableLengthInteger length;
    uint32_t packetNumber;
    // payload
}
cplusplus(ZeroRttPacketHeader) {{
  public:
    std::string str() const override {
        std::ostringstream os;
        os << "0-RTT[dstConnectionId=" << dstConnectionId
           << ", packetNumber=" << packetNumber
           << "]";
        return os.str();
    }
}}

class HandshakePacketHeader extends LongPacketHeader
{
    longPacketType = LONG_PACKET_HEADER_TYPE_HANDSHAKE;
    uint8_t packetNumberLength;
    VariableLengthInteger length;
    uint32_t packetNumber;
    // payload
}
cplusplus(HandshakePacketHeader) {{
  public:
    void calcChunkLength() {
        int64_t len = 1 + 4 + 1 + dstConnectionIdLength + 1 + srcConnectionIdLength + getVariableLengthIntegerSize(length) + packetNumberLength;
        this->setChunkLength(B(len));
    }
    
    std::string str() const override {
        std::ostringstream os;
        os << "Handshake[dstConnectionId=" << dstConnectionId
           << ", packetNumber=" << packetNumber
           << "]";
        return os.str();
    }
}}

class RetryPacketHeader extends LongPacketHeader
{
    longPacketType = LONG_PACKET_HEADER_TYPE_RETRY;
    uint64_t retryToken;
    uint64_t retryIntegrityTag;
}

class ShortPacketHeader extends PacketHeader
{
    headerForm = PACKET_HEADER_FORM_SHORT;
    uint64_t dstConnectionId;
    uint32_t packetNumber;
    // payload
}

class OneRttPacketHeader extends ShortPacketHeader
{
    bool iBit = false;
}
cplusplus(OneRttPacketHeader) {{
  public:
    static const size_t SIZE = 13;
    std::string str() const override {
        std::ostringstream os;
        os << "1-RTT[dstConnectionId=" << dstConnectionId
           << ", packetNumber=" << packetNumber
           << "]";
        return os.str();
    }
}}
