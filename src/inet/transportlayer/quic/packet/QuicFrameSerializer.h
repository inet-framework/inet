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

#ifndef __INET_QUICFRAMESERIALIZER_H
#define __INET_QUICFRAMESERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/transportlayer/quic/packet/FrameHeader_m.h"
#include "inet/transportlayer/quic/packet/VariableLengthInteger_m.h"

namespace inet {
namespace quic {

/**
 * Converts between QUIC frame headers and binary (network byte order) QUIC frame representation.
 */
class INET_API QuicFrameSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

    // Helper methods for serializing different types of QUIC frames
    virtual void serializeStreamFrame(MemoryOutputStream& stream, const Ptr<const StreamFrameHeader>& frame) const;
    virtual void serializeAckFrame(MemoryOutputStream& stream, const Ptr<const AckFrameHeader>& frame) const;
    virtual void serializeMaxDataFrame(MemoryOutputStream& stream, const Ptr<const MaxDataFrameHeader>& frame) const;
    virtual void serializeMaxStreamDataFrame(MemoryOutputStream& stream, const Ptr<const MaxStreamDataFrameHeader>& frame) const;
    virtual void serializeDataBlockedFrame(MemoryOutputStream& stream, const Ptr<const DataBlockedFrameHeader>& frame) const;
    virtual void serializeStreamDataBlockedFrame(MemoryOutputStream& stream, const Ptr<const StreamDataBlockedFrameHeader>& frame) const;
    virtual void serializePingFrame(MemoryOutputStream& stream, const Ptr<const PingFrameHeader>& frame) const;
    virtual void serializePaddingFrame(MemoryOutputStream& stream, const Ptr<const PaddingFrameHeader>& frame) const;
    virtual void serializeCryptoFrame(MemoryOutputStream& stream, const Ptr<const CryptoFrameHeader>& frame) const;
    virtual void serializeHandshakeDoneFrame(MemoryOutputStream& stream, const Ptr<const HandshakeDoneFrameHeader>& frame) const;
    virtual void serializeConnectionCloseFrame(MemoryOutputStream& stream, const Ptr<const ConnectionCloseFrameHeader>& frame) const;
    virtual void serializeTransportParametersExtension(MemoryOutputStream& stream, const Ptr<const TransportParametersExtension>& frame) const;

    // Helper methods for deserializing different types of QUIC frames
    virtual const Ptr<Chunk> deserializeFrame(MemoryInputStream& stream, uint8_t frameType) const;
    virtual const Ptr<Chunk> deserializeStreamFrame(MemoryInputStream& stream, uint8_t frameType) const;
    virtual const Ptr<Chunk> deserializeAckFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializeMaxDataFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializeMaxStreamDataFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializeDataBlockedFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializeStreamDataBlockedFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializePingFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializePaddingFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializeCryptoFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializeHandshakeDoneFrame(MemoryInputStream& stream) const;
    virtual const Ptr<Chunk> deserializeConnectionCloseFrame(MemoryInputStream& stream, uint8_t frameType) const;
    virtual const Ptr<Chunk> deserializeTransportParametersExtension(MemoryInputStream& stream) const;

    // Helper methods for variable length integers
    virtual void serializeVariableLengthInteger(MemoryOutputStream& stream, uint64_t value) const;
    virtual uint64_t deserializeVariableLengthInteger(MemoryInputStream& stream) const;

  public:
    QuicFrameSerializer() : FieldsChunkSerializer() {}
};

} // namespace quic
} // namespace inet

#endif // __INET_QUICFRAMESERIALIZER_H
