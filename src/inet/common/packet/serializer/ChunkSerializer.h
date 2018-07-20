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

#ifndef __INET_CHUNKSERIALIZER_H_
#define __INET_CHUNKSERIALIZER_H_

#include "inet/common/MemoryInputStream.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

class INET_API ChunkSerializer : public cObject
{
  public:
    static b totalSerializedLength;
    static b totalDeserializedLength;

  public:
    /**
     * Serializes a chunk into a stream by writing the bytes representing the
     * chunk at the end of the stream. The offset and length parameters allow
     * to write only a part of the data.
     */
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const = 0;

    /**
     * Deserializes a chunk from a stream by reading the bytes at the current
     * position of the stream. The returned chunk will be an instance of the
     * provided type. The current stream position is updated according to the
     * length of the returned chunk.
     */
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const = 0;
};

} // namespace

#endif // #ifndef __INET_CHUNKSERIALIZER_H_

