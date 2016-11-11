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

#ifndef __INET_SLICECHUNK_H_
#define __INET_SLICECHUNK_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

class SliceChunk : public Chunk
{
  friend Chunk;

  protected:
    std::shared_ptr<Chunk> chunk = nullptr;
    int64_t byteOffset = -1;
    int64_t byteLength = -1;

  protected:
    virtual const char *getSerializerClassName() const override { return "inet::SliceChunkSerializer"; }

  protected:
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset = -1, int64_t byteLength = -1);

  public:
    SliceChunk() { }
    SliceChunk(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset = -1, int64_t byteLength = -1);

    virtual SliceChunk *dup() const override { return new SliceChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<SliceChunk>(*this); }

    const std::shared_ptr<Chunk>& getChunk() const { return chunk; }
    void setChunk(const std::shared_ptr<Chunk>& chunk) { this->chunk = chunk; }

    int64_t getByteOffset() const { return byteOffset; }
    void setByteOffset(int64_t byteOffset);

    virtual int64_t getByteLength() const override { return byteLength; }
    void setByteLength(int64_t byteLength);

    virtual bool insertToBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool insertToEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual bool removeFromBeginning(int64_t byteLength) override;
    virtual bool removeFromEnd(int64_t byteLength) override;

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_SLICECHUNK_H_

