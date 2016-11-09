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

#ifndef __INET_BYTELENGTHCHUNK_H_
#define __INET_BYTELENGTHCHUNK_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

class ByteLengthChunk : public Chunk
{
  protected:
    int64_t byteLength = -1;

  public:
    ByteLengthChunk() { }
    ByteLengthChunk(int64_t byteLength);

    virtual int64_t getByteLength() const override { return byteLength; }
    void setByteLength(int64_t byteLength);

    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);

    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const override;

    virtual std::shared_ptr<Chunk> peek2(const Iterator& iterator, int64_t byteLength = -1) const override;

    virtual const char *getSerializerClassName() const override { return "inet::ByteLengthChunkSerializer"; }

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_BYTELENGTHCHUNK_H_

