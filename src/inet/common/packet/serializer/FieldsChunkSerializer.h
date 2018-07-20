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

#ifndef __INET_FIELDSCHUNKSERIALIZER_H_
#define __INET_FIELDSCHUNKSERIALIZER_H_

#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {

class INET_API FieldsChunkSerializer : public ChunkSerializer
{
  protected:
    /**
     * Serializes a chunk into a stream by writing all bytes representing the
     * chunk at the end of the stream.
     */
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const = 0;

    /**
     * Deserializes a chunk from a stream by reading the bytes at the current
     * position of the stream. The current stream position is updated according
     * to the length of the returned chunk.
     */
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const = 0;

  public:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const override;
};

} // namespace

#endif // #ifndef __INET_FIELDSCHUNKSERIALIZER_H_

