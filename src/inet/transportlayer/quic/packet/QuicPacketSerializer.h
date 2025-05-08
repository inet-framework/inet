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

#ifndef __INET_QUICPACKETSERIALIZER_H
#define __INET_QUICPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/transportlayer/quic/packet/PacketHeader_m.h"
#include "inet/transportlayer/quic/packet/FrameHeader_m.h"
#include "inet/transportlayer/quic/packet/VariableLengthInteger_m.h"

namespace inet {
namespace quic {

/**
 * Converts between QUIC packet headers and binary (network byte order) QUIC packet headers.
 */
class INET_API QuicPacketHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

    // Helper methods for serializing different types of QUIC packet headers
    virtual void serializeLongPacketHeader(MemoryOutputStream& stream, const Ptr<const LongPacketHeader>& header, uint8_t packetNumberLength) const;
    virtual void serializeShortPacketHeader(MemoryOutputStream& stream, const Ptr<const ShortPacketHeader>& header) const;
    virtual void serializeInitialPacketHeader(MemoryOutputStream& stream, const Ptr<const InitialPacketHeader>& header) const;
    virtual void serializeHandshakePacketHeader(MemoryOutputStream& stream, const Ptr<const HandshakePacketHeader>& header) const;
    virtual void serializeZeroRttPacketHeader(MemoryOutputStream& stream, const Ptr<const ZeroRttPacketHeader>& header) const;
    virtual void serializeRetryPacketHeader(MemoryOutputStream& stream, const Ptr<const RetryPacketHeader>& header) const;
    virtual void serializeOneRttPacketHeader(MemoryOutputStream& stream, const Ptr<const OneRttPacketHeader>& header) const;
    virtual void serializeVersionNegotiationPacketHeader(MemoryOutputStream& stream, const Ptr<const VersionNegotiationPacketHeader>& header) const;

    // Helper methods for deserializing different types of QUIC packet headers
    virtual const Ptr<Chunk> deserializeLongPacketHeader(MemoryInputStream& stream, uint8_t firstByte) const;
    virtual const Ptr<Chunk> deserializeShortPacketHeader(MemoryInputStream& stream, uint8_t firstByte) const;

  public:
    QuicPacketHeaderSerializer() : FieldsChunkSerializer() {}
};


class INET_API EncryptedQuicPacketSerializer : public ChunkSerializer
{
  protected:

    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};


} // namespace quic
} // namespace inet

#endif // __INET_QUICPACKETSERIALIZER_H
