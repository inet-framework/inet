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

#include "inet/transportlayer/quic/packet/TransportParametersExtensionSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/Endian.h"
#include "inet/transportlayer/quic/packet/VariableLengthInteger.h"

namespace inet {
namespace quic {

Register_Serializer(TransportParametersExtension, TransportParametersExtensionSerializer);

void TransportParametersExtensionSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& extension = staticPtrCast<const TransportParametersExtension>(chunk);

    // 00 39 - assigned value for extension "QUIC Transport Parameters"
    stream.writeByte(0x00); // 0x00 - assigned value for extension "QUIC Transport Parameters"
    stream.writeByte(0x39); // 0x39 - assigned value for extension "QUIC Transport Parameters"

    // 00 31 - 0x31 (49) bytes of "QUIC Transport Parameters" extension data follows
    stream.writeByte(0x00); // 0x00 - assigned value for extension "QUIC Transport Parameters"
    stream.writeByte(0x31); // 0x31 - 0x31 (49) bytes of "QUIC Transport Parameters" extension data follows

    // 04 - assigned value for "initial_max_data"
    stream.writeByte(0x04);
    // 04 - 4 bytes of "initial_max_data" data follows
    stream.writeByte(getVariableLengthIntegerSize(extension->getInitialMaxData()));
    serializeVariableLengthInteger(stream, extension->getInitialMaxData());

    // 05 - assigned value for "initial_max_stream_data_bidi_local"
    stream.writeByte(0x05);
    // 04 - 4 bytes of "initial_max_stream_data_bidi_local" data follows
    stream.writeByte(getVariableLengthIntegerSize(extension->getInitialMaxStreamDataBidiLocal()));
    // 80 10 00 00 - a variable length integer with value 0x100000 (1048576)
    serializeVariableLengthInteger(stream, extension->getInitialMaxStreamDataBidiLocal());

    // 06 - assigned value for "initial_max_stream_data_bidi_remote"
    stream.writeByte(0x06);
    // 04 - 4 bytes of "initial_max_stream_data_bidi_remote" data follows
    stream.writeByte(getVariableLengthIntegerSize(extension->getInitialMaxStreamDataBidiRemote()));
    // 80 10 00 00 - a variable length integer with value 0x100000 (1048576)
    serializeVariableLengthInteger(stream, extension->getInitialMaxStreamDataBidiRemote());

    // 07 - assigned value for "initial_max_stream_data_uni"
    stream.writeByte(0x07);
    // 04 - 4 bytes of "initial_max_stream_data_uni" data follows
    stream.writeByte(getVariableLengthIntegerSize(extension->getInitialMaxStreamDataUni()));
    // 80 10 00 00 - a variable length integer with value 0x100000 (1048576)
    serializeVariableLengthInteger(stream, extension->getInitialMaxStreamDataUni());
}

const Ptr<Chunk> TransportParametersExtensionSerializer::deserialize(MemoryInputStream& stream) const
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

void TransportParametersExtensionSerializer::serializeVariableLengthInteger(MemoryOutputStream& stream, uint64_t value) const
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

uint64_t TransportParametersExtensionSerializer::deserializeVariableLengthInteger(MemoryInputStream& stream) const
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
