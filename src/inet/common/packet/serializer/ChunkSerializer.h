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

#include "inet/common/ByteInputStream.h"
#include "inet/common/ByteOutputStream.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

class INET_API ChunkSerializer : public cObject
{
  public:
    static bit totalSerializedBitCount;
    static bit totalDeserializedBitCount;

  public:
    virtual ~ChunkSerializer() { }

    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, bit offset, bit length) const = 0;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo) const = 0;
};

} // namespace

#endif // #ifndef __INET_CHUNKSERIALIZER_H_

